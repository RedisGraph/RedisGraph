import redis
import csv
import os
import click

# Global variables (can refactor into arguments later)
max_tokens = 1024 * 1024
graphname = None
redis_client = None

# Custom error class for invalid inputs
class CSVError(Exception):
    pass

# Argument is the container class for the metadata parameters to be sent in a Redis query.
# An object of this type is comprised of the type of the inserted entity ("NODES" or "RELATIONS"),
# the number of entities, and one Descriptor for each entity.
# The 'unroll' method generates this sequence as a list to be passed as part of a Redis query
# (followed by the entities themselves).
class Argument:
    def __init__(self, argtype):
        self.argtype = argtype
        self.descriptors = []
        self.entities_created = 0
        self.queries_processed = 0
        self.total_entities = 0
        self.descriptors_in_query = 0

    def reset_tokens(self):
        for descriptor in self.descriptors:
            descriptor.reset_tokens()

    def remove_descriptors(self, delete_count):
        for descriptor in self.descriptors[:delete_count]:
            print('Finished inserting "%s".' % descriptor.name)
        del self.descriptors[:delete_count]

    def unroll(self):
        ret = [self.argtype, self.pending_inserts(), self.descriptors_in_query]
        for descriptor in self.descriptors[0:self.descriptors_in_query]:
            # Don't include a descriptor unless we are inserting its entities.
            # This can occur if the addition of a descriptor and its first entity caused
            # the token count to exceed the max allowed
            if descriptor.pending_inserts == 0:
                ret[2] -= 1
            else:
                ret += descriptor.unroll()
        return ret

    def token_count(self):
        # ["NODES"/"RELATIONS", number of entities to insert, number of different entity types, list of type descriptors]
        return 3 + sum(descriptor.token_count() for descriptor in self.descriptors[0:self.descriptors_in_query])

    def pending_inserts(self):
        return sum(desc.pending_inserts for desc in self.descriptors)

    def print_insertion_done(self):
        print("%d %s created in %d queries." % (self.entities_created, self.argtype.lower(), self.queries_processed))

    def batch_insert_descriptors(self):
        entities = [] # Tokens pending insertion from CSV rows
        token_count = 5 # Prefix tokens: "GRAPH.BULK [graphname] ["LABELS"/"RELATIONS"] [entity_count] [# of descriptors]"
        for desc in self.descriptors:
            desc.print_insertion_start()
            self.descriptors_in_query += 1
            token_count += desc.token_count()
            for row in desc.reader:
                token_count += desc.attribute_count # All rows have the same length
                if token_count > max_tokens:
                    # max_tokens has been reached; submit all but the most recent node
                    query_redis(self, entities)
                    desc.entities_created += desc.pending_inserts
                    # Reset values post-query
                    self.reset_tokens()
                    entities = []

                    # Remove descriptors that have no further pending inserts
                    # (all but the current).
                    self.remove_descriptors(self.descriptors_in_query - 1)
                    self.descriptors_in_query = 1
                    if desc.entities_created > 0:
                        desc.print_progress()
                    # Following an insertion, token_count is set to accommodate "GRAPH.BULK", graphname,
                    # all labels and attributes, plus the individual tokens from the uninserted current row
                    token_count = 2 + self.token_count() + desc.attribute_count
                desc.pending_inserts += 1
                entities += row

        # Insert all remaining nodes
        query_redis(self, entities)
        self.remove_descriptors(self.descriptors_in_query)
        self.print_insertion_done()

# The Descriptor class holds the name and element counts for an individual relationship or node label.
# After the contents of an Argument instance have been submitted to Redis in a GRAPH.BULK query,
# the contents of each descriptor contained in the query are unrolled.
# The `unroll` methods are unique to the data type.
class Descriptor:
    def __init__(self, csvfile):
        # A Label or Relationship name is set by the CSV file name
        # TODO validate name string
        self.name = os.path.splitext(os.path.basename(csvfile))[0]
        self.csvfile = open(csvfile, "rt")
        self.reader = csv.reader(self.csvfile)
        self.total_entities = 0
        self.entities_created = 0
        self.pending_inserts = 0

    def print_progress(self):
        print('%.2f%% (%d / %d) of "%s" inserted.' % (float(self.entities_created) * 100 / self.total_entities,
                                                      self.entities_created,
                                                      self.total_entities,
                                                      self.name))

    def reset_tokens(self):
        self.pending_inserts = 0

# A LabelDescriptor consists of a name string, an entity count, and a series of
# attributes (derived from the header row of the label CSV).
# As a query string, a LabelDescriptor is printed in the format:
# [label name, entity count, attribute count, attributes[0..n]]]
class LabelDescriptor(Descriptor):
    def __init__(self, csvfile):
        Descriptor.__init__(self, csvfile)
        # The first row of a label CSV contains its attributes
        self.attributes = next(self.reader)
        self.attribute_count = len(self.attributes)
        self.validate_csv()
        # Reset input CSV, then skip header line
        self.csvfile.seek(0)
        next(self.reader)

    def validate_csv(self):
        # Expect all rows to have the same column count as the header.
        expected_col_count = self.attribute_count
        for row in self.reader:
            # Raise an exception if the wrong number of columns are found
            if len(row) != expected_col_count:
                raise CSVError ("%s:%d Expected %d columns, encountered %d ('%s')"
                                % (self.csvfile, self.reader.line_num, expected_col_count, len(row), ','.join(row)))
            if (row[-1] == ''):
                raise CSVError ("%s:%d Dangling comma in input. ('%s')"
                                % (self.csvfile, self.reader.line_num, ','.join(row)))
        # Subtract 1 from each file's entity count to compensate for the header.
        self.total_entities = self.reader.line_num - 1

    def token_count(self):
        # Labels have a token for name, attribute count, pending insertion count, plus N tokens for the individual attributes.
        return 3 + self.attribute_count

    def unroll(self):
        return [self.name, self.pending_inserts, self.attribute_count] + self.attributes

    def print_insertion_start(self):
        print('Inserting Label "%s" - %d nodes' % (self.name, self.total_entities))

# A RelationDescriptor consists of a name string and a relationship count.
# As a query string, a RelationDescriptor is printed in the format:
# [relation name, relation count]
class RelationDescriptor(Descriptor):
    def __init__(self, csvfile):
        Descriptor.__init__(self, csvfile)
        self.attribute_count = 2
        self.validate_csv()
        self.csvfile.seek(0)

    def validate_csv(self):
        for row in self.reader:
            # Each row should have two columns (a source and dest ID)
            if len(row) != 2:
                raise CSVError ("%s:%d Expected 2 columns, encountered %d ('%s')"
                                % (self.csvfile, self.reader.line_num, len(row), ','.join(row)))
            for elem in row:
                # Raise an exception if an element cannot be read as an integer
                try:
                    int(elem)
                except:
                    raise CSVError ("%s:%d Token '%s' was not a node ID)"
                            % (self.csvfile, self.reader.line_num, elem))
        self.total_entities = self.reader.line_num

    def token_count(self):
        # Relations have 2 tokens: name and pending insertion count.
        return 2

    def unroll(self):
        return [self.name, self.pending_inserts]

    def print_insertion_start(self):
        print('Inserting relation "%s" - %d edges' % (self.name, self.total_entities))

# Issue single Redis query to allocate space for graph
def allocate_graph(node_count, relation_count):
    cmd = ["GRAPH.BULK", graphname, "BEGIN", node_count, relation_count]
    result = redis_client.execute_command(*cmd)
    print(result)

def finalize_graph():
    cmd = ["GRAPH.BULK", graphname, "END"]
    result = redis_client.execute_command(*cmd)
    print(result)

def query_redis(metadata, entities):
    cmd = ["GRAPH.BULK", graphname] + metadata.unroll() + entities
    # Raise error if query doesn't contain entities
    if not entities:
        raise Exception ("Attempted to insert 0 tokens( '%s')." % (" ".join(str(e) for e in cmd)))

    # Send query to Redis client
    result = redis_client.execute_command(*cmd)
    stats = result.split(', '.encode())
    metadata.entities_created += int(stats[0].split(' '.encode())[0])
    metadata.entities_created += int(stats[1].split(' '.encode())[0])
    metadata.queries_processed += 1

def build_descriptors(csvs, argtype):
    # Prepare container for all labels
    arg = Argument(argtype)

    # Generate a label descriptor from each given label CSV
    for f in csvs:
        # Better method for this?
        if (argtype == "NODES"):
            descriptor = LabelDescriptor(f)
        else:
            descriptor = RelationDescriptor(f)
        arg.descriptors.append(descriptor)
    arg.total_entities = sum(desc.total_entities for desc in arg.descriptors)
    return arg

def help():
    pass

# Command-line arguments
@click.command()
@click.argument('graph')
# Redis server connection settings
@click.option('--host', '-h', default='127.0.0.1', help='Redis server host')
@click.option('--port', '-p', default=6379, help='Redis server port')
@click.option('--password', '-P', default=None, help='Redis server password')
@click.option('--ssl', '-s', default=False, help='Server is SSL-enabled')
# CSV file paths
@click.option('--nodes', '-n', required=True, multiple=True, help='path to node csv file')
@click.option('--relations', '-r', multiple=True, help='path to relation csv file')
# Debug options
@click.option('--max_buffer_size', '-m', default=1024*1024, help='(DEBUG ONLY) - max token count per Redis query')

def bulk_insert(graph, host, port, password, ssl, nodes, relations, max_buffer_size):
    global graphname
    global redis_client
    global max_tokens
    graphname = graph
    redis_client = redis.StrictRedis(host=host, port=port, password=password, ssl=ssl)
    if max_buffer_size > max_tokens:
        print("Requested buffer size too large, capping queries at %d." % max_tokens)
    else:
        max_tokens = max_buffer_size

    # Iterate over label CSVs to validate inputs and build label descriptors.
    print("Building label descriptors...")
    label_descriptors = build_descriptors(nodes, "NODES")
    relation_count = 0
    if relations:
        relation_descriptors = build_descriptors(relations, "RELATIONS")
        relation_count = relation_descriptors.total_entities

    # Send prefix tokens to RedisGraph
    # This could be also done as part of the first query,
    # but would somewhat complicate counting logic
    allocate_graph(label_descriptors.total_entities, relation_count)

    # Process input CSVs and commit their contents to RedisGraph
    # Possibly make this a method on Argument
    label_descriptors.batch_insert_descriptors()
    if relations:
        relation_descriptors.batch_insert_descriptors()
    finalize_graph()

if __name__ == '__main__':
    bulk_insert()

