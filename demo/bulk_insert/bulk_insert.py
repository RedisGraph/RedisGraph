import csv
import os
import struct
import redis
import click

# Global variables
CONFIGS = None # thresholds for batching Redis queries
NODE_DICT = [] # global node dictionary
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
class Configs:
    def __init__(self, max_token_count, max_buffer_size, max_token_size):
        # Maximum number of tokens per query
        # 1024 * 1024 is a hard-coded Redis maximum
        self.max_token_count = min(max_token_count, 1024 * 1024)
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
        self.prefix_args = [graphname, "BEGIN"]

    # For each node input file, validate contents and convert to binary format.
    # If any buffer limits have been reached, flush all enqueued inserts to Redis.
    def process_entity_csvs(self, cls, csvs):
        for in_csv in csvs:
            # Build entity descriptor from input CSV
            entity = cls(in_csv)
            added_size = entity.binary_size
            # Check to see if the addition of this data will exceed the buffer's capacity
            if (self.buffer_size + added_size > CONFIGS.max_buffer_size
                    or self.redis_token_count + 1 > CONFIGS.max_token_count):
                # Send and flush the buffer if appropriate
                self.send_buffer()
            # Add binary data to list and update all counts
            self.redis_token_count += 1
            self.buffer_size += added_size

    def send_buffer(self):
        # Do nothing if we have no entities
        if Label.node_count == 0 and RelationType.relation_count == 0:
            return

        args = self.prefix_args + [Label.node_count, RelationType.relation_count]

        if Label.labels:
            args += ["NODES"] + [e.to_binary() for e in Label.labels]

        if RelationType.reltypes:
            args += ["RELATIONS"] + [e.to_binary() for e in RelationType.reltypes]
        result = self.client.execute_command("GRAPH.BULK", *args)
        print(result)

        del self.prefix_args[1:] # Delete "BEGIN" token if present

        self.clear_buffer()

    def clear_buffer(self):
        self.redis_token_count = 0
        self.buffer_size = 0

        # All constructed entities have been inserted, so clear buffers
        Label.node_count = 0
        RelationType.relation_count = 0
        del Label.labels[:]
        del RelationType.reltypes[:]

# Property is a class for converting a single CSV property field into a binary stream.
# Supported property types are string, numeric, boolean, and NULL.
class Property(object):
    def __init__(self, prop_str):
        # All format strings start with an unsigned char to represent our Type enum
        self.format_str = "=B"

        if not prop_str:
            # An empty field indicates a NULL property
            self.type = Type.NULL
            self.pack_args = []
            return

        # If field can be cast to a float, allow it
        try:
            self.pack_args = [float(prop_str)]
            self.type = Type.NUMERIC
            self.format_str += "d"
            return
        except:
            pass

        # If field is 'false' or 'true', it is a boolean
        if prop_str.lower() == 'false':
            self.type = Type.BOOL
            self.format_str += "?"
            self.pack_args = [False]
            return
        if prop_str.lower() == 'true':
            self.type = Type.BOOL
            self.format_str += "?"
            self.pack_args = [True]
            return

        # If we've reached this point, the property is a string
        self.type = Type.STRING
        # Encoding len+1 adds a null terminator to the string
        self.format_str += "%ds" % (len(prop_str) + 1)
        self.pack_args = [str.encode(prop_str)]

    def to_binary(self):
        return struct.pack(self.format_str, *[self.type] + self.pack_args)


# Superclass for label and relation CSV files
class EntityFile(object):
    def __init__(self, filename):
        # The label or relation type string is the basename of the file
        self.entity_str = os.path.splitext(os.path.basename(filename))[0].encode("ascii")
        # Input file handling
        self.infile = open(filename, 'rt')
        # Initialize CSV reader that ignores leading whitespace in each field
        # and does not modify input quote characters
        self.reader = csv.reader(self.infile, skipinitialspace=True, quoting=csv.QUOTE_NONE)

        self.prop_offset = 0 # Starting index of properties in row
        self.prop_count = 0 # Number of properties per entity
        #  self.entity_count = 0 # Total number of entities

        self.packed_header = ""
        self.binary_entities = []
        self.binary_size = 0 # size of binary token

    # entity_string refers to label or relation type string
    def pack_header(self, header):
        prop_count = len(header) - self.prop_offset
        # String format
        fmt = "=%dsI" % (len(self.entity_str) + 1) # Unaligned native, entity_string, count of properties
        args = [self.entity_str, prop_count]
        for prop in header[self.prop_offset:]:
            fmt += "%ds" % (len(prop) + 1)
            args += [str.encode(prop)]
        return struct.pack(fmt, *args)

    def pack_props(self, line):
        props = []
        for field in line[self.prop_offset:]:
            props.append(Property(field))

        return b''.join(p.to_binary() for p in props)

    def to_binary(self):
        return self.packed_header + b''.join(self.binary_entities)

# Handler class for processing label csv files.
class Label(EntityFile):
    node_count = 0 # Track number of pending node inserts across all labels
    labels = []

    def __init__(self, infile):
        super(Label, self).__init__(infile)
        expected_col_count = self.process_header()
        self.process_entities(expected_col_count)
        self.infile.close()

    def process_header(self):
        # Header format:
        # source identifier, dest identifier, properties[0..n]
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
        Label.labels.append(self)
        for row in self.reader:
            # Expect all entities to have the same property count
            if len(row) != expected_col_count:
                raise CSVError("%s:%d Expected %d columns, encountered %d ('%s')"
                               % (self.infile.name, self.reader.line_num, expected_col_count, len(row), ','.join(row)))
            # Check for dangling commma
            if row[-1] == ',':
                raise CSVError("%s:%d Dangling comma in input. ('%s')"
                               % (self.infile.name, self.reader.line_num, ','.join(row)))
            # Add identifier->ID pair to dictionary if we are building relations
            if NODE_DICT is not None: # TODO "is not None" - necessary?
                if row[0] in NODE_DICT:
                    print("Node identifier '%s' was used multiple times - second occurrence at %s:%d"
                          % (row[0], self.infile.name, self.reader.line_num))
                NODE_DICT[row[0]] = TOP_NODE_ID
                TOP_NODE_ID += 1
            row_binary = self.pack_props(row)
            row_binary_len = len(row_binary)
            # If the addition of this entity will the binary token too large, send the buffer now.
            if self.binary_size + row_binary_len > CONFIGS.max_token_size:
                QUERY_BUF.send_buffer()
                # TODO improve this
                # Maybe move class variables to query buffer instance variables
                # Delete just-inserted entities and add self again, as the query buffer has been flushed
                self.binary_entities = []
                self.binary_size = len(self.packed_header)
                Label.labels.append(self)

            #  self.entity_count += 1
            Label.node_count += 1
            self.binary_size += row_binary_len
            self.binary_entities.append(row_binary)

# Handler class for processing relation csv files.
class RelationType(EntityFile):
    relation_count = 0 # Track number of pending relation inserts across all types
    reltypes = []

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
        RelationType.reltypes.append(self)
        for row in self.reader:
            # Each row should have the same number of fields
            if len(row) != expected_col_count:
                raise CSVError("%s:%d Expected %d columns, encountered %d ('%s')"
                               % (self.infile.name, self.reader.line_num, expected_col_count, len(row), ','.join(row)))
            # Check for dangling commma
            if  row[-1] == '':
                raise CSVError("%s:%d Dangling comma in input. ('%s')"
                               % (self.infile.name, self.reader.line_num, ','.join(row)))

            src = NODE_DICT[row[0]]
            dest = NODE_DICT[row[1]]
            fmt = "=QQ" # 8-byte unsigned ints for src and dest
            row_binary = struct.pack(fmt, src, dest) + self.pack_props(row)
            row_binary_len = len(row_binary)
            # If the addition of this entity will the binary token too large, send the buffer now.
            if self.binary_size + row_binary_len > CONFIGS.max_token_size:
                QUERY_BUF.send_buffer()
                # Delete just-inserted entities and add self again, as the query buffer has been flushed
                self.binary_entities = []
                self.binary_size = len(self.packed_header)
                RelationType.reltypes.append(self)

            #  self.entity_count += 1
            RelationType.relation_count += 1
            self.binary_size += row_binary_len
            self.binary_entities.append(row_binary)

def help():
    pass

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
@click.option('--max-token-count', '-t', default=1024, help='max number of tokens to send per query (default 1024)')
@click.option('--max-buffer-size', '-b', default=4096, help='max buffer size in megabytes (default 4096)')
@click.option('--max-token-size', '-b', default=500, help='max size of each token in megabytes (default 500, max 512)')

def bulk_insert(graph, host, port, password, nodes, relations, max_token_count, max_buffer_size, max_token_size):
    global CONFIGS
    global NODE_DICT
    global TOP_NODE_ID
    global QUERY_BUF

    TOP_NODE_ID = 0 # reset global variable

    CONFIGS = Configs(max_token_count, max_buffer_size, max_token_size)

    # Connect to Redis server and initialize buffer
    client = redis.StrictRedis(host=host, port=port, password=password)

    QUERY_BUF = QueryBuffer(graph, client)
    QUERY_BUF.clear_buffer() # Reset class variables (in case we're in unit tests)

    # Create a node dictionary if we're building relations and as such require unique identifiers
    if relations:
        NODE_DICT = {}
    else:
        NODE_DICT = None

    QUERY_BUF.process_entity_csvs(Label, nodes)

    if relations:
        QUERY_BUF.process_entity_csvs(RelationType, relations)

    # Send all remaining tokens to Redis
    QUERY_BUF.send_buffer()

if __name__ == '__main__':
    bulk_insert()
