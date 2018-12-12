import csv
import os
import struct
import redis
import click

# Global variables
CONFIGS = None # thresholds for batching Redis queries
NODE_DICT = [] # global node dictionary
TOP_NODE_ID = 0 # next ID to assign to a node

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
        self.node_count = 0 # number of distinct nodes currently in buffer
        self.relation_count = 0 # number of distinct relations currently in buffer

        # Sizes for buffer currently being constructed
        self.redis_token_count = 0
        self.buffer_size = 0

        # The first query should include a "BEGIN" token
        self.prefix_args = [graphname, "BEGIN"]

        # Containers for binary blobs
        self.label_blobs = []
        self.reltype_blobs = []

    # For each node input file, validate contents and convert to binary format.
    # If any buffer limits have been reached, flush all enqueued inserts to Redis.
    def process_node_csvs(self, csvs):
        for in_csv in csvs:
            # Build Label descriptor from input CSV
            label = Label(in_csv)
            label_blob = label.to_binary()
            added_size = len(label_blob)
            # Check to see if the addition of this data will exceed the buffer's capacity
            if (self.buffer_size + added_size > CONFIGS.max_buffer_size
                    or self.redis_token_count + 1 > CONFIGS.max_token_count):
                # Send and flush the buffer if appropriate
                self.send_buffer()
            # Add binary data to list and update all counts
            self.label_blobs.append(label_blob)
            self.node_count += label.entity_count
            self.redis_token_count += 1
            self.buffer_size += added_size

    # This function is the relations equivalent of process_node_csvs.
    # TODO it would be nice to consolidate them later.
    def process_relation_csvs(self, csvs):
        for in_csv in csvs:
            # Build RelationType descriptor from input CSV
            rel = RelationType(in_csv)
            reltype_blob = rel.to_binary()
            added_size = len(reltype_blob)
            # Check to see if the addition of this data will exceed the buffer's capacity
            if (self.buffer_size + added_size > CONFIGS.max_buffer_size
                    or self.redis_token_count + 1 > CONFIGS.max_token_count):
                # Send and flush the buffer if appropriate
                self.send_buffer()
            # Add binary data to list and update all counts
            self.reltype_blobs.append(reltype_blob)
            self.relation_count += rel.entity_count
            self.redis_token_count += 1
            self.buffer_size += added_size

    def send_buffer(self):
        args = self.prefix_args + [self.node_count, self.relation_count]
        if self.label_blobs:
            args += ["NODES"] + self.label_blobs

        if self.reltype_blobs:
            args += ["RELATIONS"] + self.reltype_blobs

        result = self.client.execute_command("GRAPH.BULK", *args)
        print(result)

        del self.prefix_args[1:] # Delete "BEGIN" token if present

        self.clear_buffer()

    def clear_buffer(self):
        self.node_count = 0
        self.relation_count = 0

        self.redis_token_count = 0
        self.buffer_size = 0

        del self.label_blobs[:]
        del self.reltype_blobs[:]

# Property is a class for converting a single CSV property field into a binary stream.
# Supported property types are string, numeric, boolean, and NULL.
class Property(object):
    def __init__(self, prop_str):
        self.orig = prop_str
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
        if self.orig.lower() == 'false':
            self.type = Type.BOOL
            self.format_str += "?"
            self.pack_args = [False]
            return
        if self.orig.lower() == 'true':
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

class EntityContainer(object):
    def __init__(self):
        self.blobs = []
        self.blob_count = 0
        self.total_size = 0
        self.entity_count = 0

    def add_entity(self, cls, infile):
        entity = cls(infile)
        blob = entity.to_binary()
        added_size = len(blob)
        # Check to see if the addition of this data will exceed the buffer's capacity
        if (self.buffer_size + added_size > CONFIGS.max_buffer_size
                or self.redis_token_count + 1 > CONFIGS.max_token_count):
            # Send and flush the buffer if appropriate
            self.send_buffer()
        # Add binary data to list and update all counts
        self.label_blobs.append(label_blob)
        self.node_count += label.entity_count
        self.redis_token_count += 1
        self.buffer_size += added_size

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
        self.entity_count = 0 # Total number of entities

        self.packed_header = ""
        self.entities = []
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
        return self.packed_header + b''.join(self.entities)

# Handler class for processing label csv files.
class Label(EntityFile):
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
                # flush buffer
                pass

            self.entity_count += 1
            self.entities.append(row_binary)

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
            # If the addition of this entity will the binary token too large, send the buffer now.

            self.entity_count += 1
            self.entities.append(struct.pack(fmt, src, dest) + self.pack_props(row))

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
    TOP_NODE_ID = 0 # reset global variable

    CONFIGS = Configs(max_token_count, max_buffer_size, max_token_size)

    # Connect to Redis server and initialize buffer
    client = redis.StrictRedis(host=host, port=port, password=password)

    query_buf = QueryBuffer(graph, client)
    # Create a node dictionary if we're building relations and as such require unique identifiers
    if relations:
        NODE_DICT = {}
    else:
        NODE_DICT = None

    query_buf.process_node_csvs(nodes)

    if relations:
        query_buf.process_relation_csvs(relations)

    # Send all remaining tokens to Redis
    query_buf.send_buffer()

if __name__ == '__main__':
    bulk_insert()
