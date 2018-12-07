import csv
import os
import struct
import redis
import click

NODE_COUNT = 0
RELATION_COUNT = 0
NODE_DICT = {}

# Custom error class for invalid inputs
class CSVError(Exception):
    pass

# Official enum support varies widely between 2.7 and 3.x, so we'll use a custom class
class Type:
    NULL = 0
    BOOL = 1
    NUMERIC = 2
    STRING = 3

# String, numeric, boolean, or NULL
class Property:
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
        self.format_str += "%ds" % (len(prop_str) + 1)
        self.pack_args = [str.encode(prop_str)]

    def to_binary(self):
        return struct.pack(self.format_str, *[self.type] + self.pack_args)

class EntityFile(object):
    def __init__(self, filename):
        # The label or relation type string is the basename of the file
        self.entity_str = os.path.splitext(os.path.basename(filename))[0].encode("ascii")
        # Input file handling
        self.infile = open(filename, 'rt')
        # Initialize CSV reader that ignores leading whitespace in each field and does not modify input quote characters
        self.reader = csv.reader(self.infile, skipinitialspace=True, quoting=csv.QUOTE_NONE)

        self.prop_offset = 0 # Starting index of properties in row
        self.expected_col_count = 0
        self.prop_count = 0 # Number of properties per entity

        self.entity_count = 0 # Total number of entities

        self.packed_header = ""
        self.entities = []

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

class NodeFile(EntityFile):
    def __init__(self, infile):
        super(NodeFile, self).__init__(infile)
        self.process_header()
        self.process_entities()
        self.infile.close()

    def process_header(self):
        # Header format:
        # source identifier, dest identifier, properties[0..n]
        header = next(self.reader)
        self.expected_col_count = len(header)
        # If identifier field begins with an underscore, don't add it as a property.
        if header[0][0] == '_':
            self.prop_offset = 1
        self.packed_header = self.pack_header(header)

    def process_entities(self):
        global NODE_DICT
        global NODE_COUNT

        for row in self.reader:
            # Expect all entities to have the same property count
            if len(row) != self.expected_col_count:
                raise CSVError("%s:%d Expected %d columns, encountered %d ('%s')"
                               % (self.infile.name, self.reader.line_num, self.expected_col_count, len(row), ','.join(row)))
            # Check for dangling commma
            if row[-1] == ',':
                raise CSVError("%s:%d Dangling comma in input. ('%s')"
                               % (self.infile.name, self.reader.line_num, ','.join(row)))
            # Add identifier->ID pair to dictionary
            if row[0] in NODE_DICT:
                print("Node identifier '%s' was used multiple times - second occurrence at %s:%d" % (row[0], self.infile.name, self.reader.line_num))
            NODE_DICT[row[0]] = NODE_COUNT
            NODE_COUNT += 1
            self.entity_count += 1
            self.entities.append(self.pack_props(row))


class RelationFile(EntityFile):
    def __init__(self, infile):
        super(RelationFile, self).__init__(infile)
        self.process_header()
        self.process_entities()
        self.infile.close()

    def process_header(self):
        # Header format:
        # source identifier, dest identifier, properties[0..n]
        header = next(self.reader)
        # Assume rectangular CSVs
        self.expected_col_count = len(header)
        self.prop_count = self.expected_col_count - 2
        if self.prop_count < 0:
            print("Relation file '%s' should have at least 2 elements in header line." % (self.infile.name))
            return # TODO return?

        self.prop_offset = 2
        self.packed_header = self.pack_header(header) # skip src and dest identifiers

    def process_entities(self):
        global RELATION_COUNT
        for row in self.reader:
            # Each row should have the same number of fields
            if len(row) != self.expected_col_count:
                raise CSVError("%s:%d Expected %d columns, encountered %d ('%s')"
                               % (self.infile.name, self.reader.line_num, self.expected_col_count, len(row), ','.join(row)))
            # Check for dangling commma
            if  row[-1] == '':
                raise CSVError("%s:%d Dangling comma in input. ('%s')"
                               % (self.infile.name, self.reader.line_num, ','.join(row)))

            src = NODE_DICT[row[0]]
            dest = NODE_DICT[row[1]]
            fmt = "=QQ" # 8-byte unsigned ints for src and dest
            self.entity_count += 1
            RELATION_COUNT += 1
            self.entities.append(struct.pack(fmt, src, dest) + self.pack_props(row))

def process_node_csvs(csvs):
    nodefiles = []
    for in_csv in csvs:
        nodefiles.append(NodeFile(in_csv))
    return nodefiles

def process_relation_csvs(csvs):
    relfiles = []
    for in_csv in csvs:
        relfiles.append(RelationFile(in_csv))
    return relfiles


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
@click.option('--nodes', '-n', required=True, multiple=True, help='path to node csv file')
@click.option('--relations', '-r', multiple=True, help='path to relation csv file')

def bulk_insert(graph, host, port, password, nodes, relations):
    nodefiles = process_node_csvs(nodes)

    if relations:
        relfiles = process_relation_csvs(relations)

    args = [graph, NODE_COUNT, RELATION_COUNT, "NODES"] + [e.to_binary() for e in nodefiles]

    if RELATION_COUNT > 0:
        args += ["RELATIONS"] + [e.to_binary() for e in relfiles]

    redis_client = redis.StrictRedis(host=host, port=port, password=password)
    #  print(args)
    result = redis_client.execute_command("GRAPH.BULK", *args)
    print(result)

if __name__ == '__main__':
    bulk_insert()
