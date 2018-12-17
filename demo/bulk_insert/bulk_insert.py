import csv
import os
import io
import struct
import redis
import click
from backports import csv

# Global variables
CONFIGS = None # thresholds for batching Redis queries
NODE_DICT = {} # global node dictionary
TOP_NODE_ID = 0 # next ID to assign to a node
QUERY_BUF = None # Buffer for query being constructed

# Custom error class for invalid inputs
class CSVError(Exception):
    pass

# Official enum support varies widely between 2.7 and 3.x, so we'll use a custom class
class Type:
    NULL = 0
    BOOL = 1
    NUMERIC = 2
    STRING = 3

# User-configurable thresholds for when to send queries to Redis
class Configs(object):
    def __init__(self, max_token_count, max_buffer_size, max_token_size):
        # Maximum number of tokens per query
        # 1024 * 1024 is the hard-coded Redis maximum. We'll set a slightly lower limit so
        # that we can safely ignore tokens that aren't binary strings
        # ("GRAPH.BULK", "BEGIN", graph name, counts)
        self.max_token_count = min(max_token_count, 1024 * 1023)
        # Maximum size in bytes per query
        self.max_buffer_size = max_buffer_size * 1000000
        # Maximum size in bytes per token
        # 512 megabytes is a hard-coded Redis maximum
        self.max_token_size = min(max_token_size * 1000000, 512 * 1000000)

# QueryBuffer is the class that processes input CSVs and emits their binary formats to the Redis client.
class QueryBuffer(object):
    def __init__(self, graphname, client):
        # Redis client and data for each query
        self.client = client

        # Sizes for buffer currently being constructed
        self.redis_token_count = 0
        self.buffer_size = 0

        # The first query should include a "BEGIN" token
        self.graphname = graphname
        self.initial_query = True

        self.node_count = 0
        self.relation_count = 0

        self.labels = [] # List containing all pending Label objects
        self.reltypes = [] # List containing all pending RelationType objects

    # Send all pending inserts to Redis
    def send_buffer(self):
        # Do nothing if we have no entities
        if self.node_count == 0 and self.relation_count == 0:
            return

        args = [self.node_count, self.relation_count, len(self.labels), len(self.reltypes)] + self.labels + self.reltypes
        # Prepend a "BEGIN" token if this is the first query
        if self.initial_query:
            args.insert(0, "BEGIN")
            self.initial_query = False

        result = self.client.execute_command("GRAPH.BULK", self.graphname, *args)
        print(result)

        self.clear_buffer()

    # Delete all entities that have been inserted
    def clear_buffer(self):
        self.redis_token_count = 0
        self.buffer_size = 0

        # All constructed entities have been inserted, so clear buffers
        self.node_count = 0
        self.relation_count = 0
        del self.labels[:]
        del self.reltypes[:]

# Superclass for label and relation CSV files
class EntityFile(object):
    def __init__(self, filename):
        # The label or relation type string is the basename of the file
        self.entity_str = os.path.splitext(os.path.basename(filename))[0].encode('utf-8')
        # Input file handling
        self.infile = io.open(filename, 'rt', encoding='utf-8')
        # Initialize CSV reader that ignores leading whitespace in each field
        # and does not modify input quote characters
        self.reader = csv.reader(self.infile, skipinitialspace=True, quoting=csv.QUOTE_NONE)

        self.prop_offset = 0 # Starting index of properties in row
        self.prop_count = 0 # Number of properties per entity

        self.packed_header = ""
        self.binary_entities = []
        self.binary_size = 0 # size of binary token

    # Simple input validations for each row of a CSV file
    def validate_row(self, expected_col_count, row):
        # Each row should have the same number of fields
        if len(row) != expected_col_count:
            raise CSVError("%s:%d Expected %d columns, encountered %d ('%s')"
                           % (self.infile.name, self.reader.line_num, expected_col_count, len(row), ','.join(row)))

    # If part of a CSV file was sent to Redis, delete the processed entities and update the binary size
    def reset_partial_binary(self):
        self.binary_entities = []
        self.binary_size = len(self.packed_header)

    # Convert property keys from a CSV file header into a binary string
    def pack_header(self, header):
        prop_count = len(header) - self.prop_offset
        # String format
        fmt = "=%dsI" % (len(self.entity_str) + 1) # Unaligned native, entity_string, count of properties
        args = [self.entity_str, prop_count]
        for p in header[self.prop_offset:]:
            prop = p.encode('utf-8')
            fmt += "%ds" % (len(prop) + 1) # encode string with a null terminator
            args.append(prop)
        return struct.pack(fmt, *args)

    # Convert a list of properties into a binary string
    def pack_props(self, line):
        props = []
        for field in line[self.prop_offset:]:
            props.append(prop_to_binary(field))

        return b''.join(p for p in props)

    def to_binary(self):
        return self.packed_header + b''.join(self.binary_entities)

# Handler class for processing label csv files.
class Label(EntityFile):
    def __init__(self, infile):
        super(Label, self).__init__(infile)
        expected_col_count = self.process_header()
        self.process_entities(expected_col_count)
        self.infile.close()

    def process_header(self):
        # Header format:
        # node identifier (which may be a property key), then all other property keys
        header = next(self.reader)
        expected_col_count = len(header)
        # If identifier field begins with an underscore, don't add it as a property.
        if header[0][0] == '_':
            self.prop_offset = 1
        self.packed_header = self.pack_header(header)
        self.binary_size += len(self.packed_header)
        return expected_col_count

    def process_entities(self, expected_col_count):
        global NODE_DICT
        global TOP_NODE_ID
        global QUERY_BUF

        for row in self.reader:
            self.validate_row(expected_col_count, row)
            # Add identifier->ID pair to dictionary if we are building relations
            if NODE_DICT is not None:
                if row[0] in NODE_DICT:
                    print("Node identifier '%s' was used multiple times - second occurrence at %s:%d"
                          % (row[0], self.infile.name, self.reader.line_num))
                    exit(1)
                NODE_DICT[row[0]] = TOP_NODE_ID
                TOP_NODE_ID += 1
            row_binary = self.pack_props(row)
            row_binary_len = len(row_binary)
            # If the addition of this entity will make the binary token grow too large,
            # send the buffer now.
            if self.binary_size + row_binary_len > CONFIGS.max_token_size:
                QUERY_BUF.labels.append(self.to_binary())
                QUERY_BUF.send_buffer()
                self.reset_partial_binary()
                # Push the label onto the query buffer again, as there are more entities to process.
                QUERY_BUF.labels.append(self.to_binary())

            QUERY_BUF.node_count += 1
            self.binary_size += row_binary_len
            self.binary_entities.append(row_binary)
        QUERY_BUF.labels.append(self.to_binary())

# Handler class for processing relation csv files.
class RelationType(EntityFile):
    def __init__(self, infile):
        super(RelationType, self).__init__(infile)
        expected_col_count = self.process_header()
        self.process_entities(expected_col_count)
        self.infile.close()

    def process_header(self):
        # Header format:
        # source identifier, dest identifier, properties[0..n]
        header = next(self.reader)
        # Assume rectangular CSVs
        expected_col_count = len(header)
        self.prop_count = expected_col_count - 2
        if self.prop_count < 0:
            raise CSVError("Relation file '%s' should have at least 2 elements in header line."
                           % (self.infile.name))

        self.prop_offset = 2
        self.packed_header = self.pack_header(header) # skip src and dest identifiers
        self.binary_size += len(self.packed_header)
        return expected_col_count

    def process_entities(self, expected_col_count):
        for row in self.reader:
            self.validate_row(expected_col_count, row)

            try:
                src = NODE_DICT[row[0]]
                dest = NODE_DICT[row[1]]
            except KeyError as e:
                print("Relationship specified a non-existent identifier.")
                raise e
            fmt = "=QQ" # 8-byte unsigned ints for src and dest
            row_binary = struct.pack(fmt, src, dest) + self.pack_props(row)
            row_binary_len = len(row_binary)
            # If the addition of this entity will make the binary token grow too large,
            # send the buffer now.
            if self.binary_size + row_binary_len > CONFIGS.max_token_size:
                QUERY_BUF.reltypes.append(self.to_binary())
                QUERY_BUF.send_buffer()
                self.reset_partial_binary()
                # Push the reltype onto the query buffer again, as there are more entities to process.
                QUERY_BUF.reltypes.append(self.to_binary())

            QUERY_BUF.relation_count += 1
            self.binary_size += row_binary_len
            self.binary_entities.append(row_binary)
        QUERY_BUF.reltypes.append(self.to_binary())

# Convert a single CSV property field into a binary stream.
# Supported property types are string, numeric, boolean, and NULL.
def prop_to_binary(prop_str):
    # All format strings start with an unsigned char to represent our Type enum
    format_str = "=B"
    if not prop_str:
        # An empty field indicates a NULL property
        return struct.pack(format_str, Type.NULL)

    # If field can be cast to a float, allow it
    try:
        numeric_prop = float(prop_str)
        return struct.pack(format_str + "d", Type.NUMERIC, numeric_prop)
    except:
        pass

    # If field is 'false' or 'true', it is a boolean
    if prop_str.lower() == 'false':
        return struct.pack(format_str + '?', Type.BOOL, False)
    elif prop_str.lower() == 'true':
        return struct.pack(format_str + '?', Type.BOOL, True)

    # If we've reached this point, the property is a string
    # Encoding len+1 adds a null terminator to the string
    encoded_str = prop_str.encode('utf-8')
    format_str += "%ds" % (len(encoded_str) + 1)
    return struct.pack(format_str, Type.STRING, encoded_str)

# For each node input file, validate contents and convert to binary format.
# If any buffer limits have been reached, flush all enqueued inserts to Redis.
def process_entity_csvs(cls, csvs):
    global QUERY_BUF
    for in_csv in csvs:
        # Build entity descriptor from input CSV
        entity = cls(in_csv)
        added_size = entity.binary_size
        # Check to see if the addition of this data will exceed the buffer's capacity
        if (QUERY_BUF.buffer_size + added_size >= CONFIGS.max_buffer_size
                or QUERY_BUF.redis_token_count + len(entity.binary_entities) >= CONFIGS.max_token_count):
            # Send and flush the buffer if appropriate
            QUERY_BUF.send_buffer()
        # Add binary data to list and update all counts
        QUERY_BUF.redis_token_count += len(entity.binary_entities)
        QUERY_BUF.buffer_size += added_size

# Command-line arguments
@click.command()
@click.argument('graph')
# Redis server connection settings
@click.option('--host', '-h', default='127.0.0.1', help='Redis server host')
@click.option('--port', '-p', default=6379, help='Redis server port')
@click.option('--password', '-a', default=None, help='Redis server password')
# CSV file paths
@click.option('--nodes', '-n', required=True, multiple=True, help='Path to node csv file')
@click.option('--relations', '-r', multiple=True, help='Path to relation csv file')
# Buffer size restrictions
@click.option('--max-token-count', '-c', default=1024, help='max number of processed CSVs to send per query (default 1024)')
@click.option('--max-buffer-size', '-b', default=4096, help='max buffer size in megabytes (default 4096)')
@click.option('--max-token-size', '-t', default=500, help='max size of each token in megabytes (default 500, max 512)')

def bulk_insert(graph, host, port, password, nodes, relations, max_token_count, max_buffer_size, max_token_size):
    global CONFIGS
    global NODE_DICT
    global TOP_NODE_ID
    global QUERY_BUF

    TOP_NODE_ID = 0 # reset global ID variable (in case we are calling bulk_insert from unit tests)

    CONFIGS = Configs(max_token_count, max_buffer_size, max_token_size)

    # Attempt to connect to Redis server
    try:
        client = redis.StrictRedis(host=host, port=port, password=password)
    except redis.exceptions.ConnectionError as e:
        print("Could not connect to Redis server.")
        raise e

    # Attempt to verify that RedisGraph module is loaded
    try:
        module_list = client.execute_command("MODULE LIST")
        if not any(b'graph' in module_description for module_description in module_list):
            print("RedisGraph module not loaded on connected server.")
            exit(1)
    except redis.exceptions.ResponseError:
        # Ignore check if the connected server does not support the "MODULE LIST" command
        pass

    QUERY_BUF = QueryBuffer(graph, client)

    # Create a node dictionary if we're building relations and as such require unique identifiers
    if relations:
        NODE_DICT = {}
    else:
        NODE_DICT = None

    process_entity_csvs(Label, nodes)

    if relations:
        process_entity_csvs(RelationType, relations)

    # Send all remaining tokens to Redis
    QUERY_BUF.send_buffer()

if __name__ == '__main__':
    bulk_insert()
