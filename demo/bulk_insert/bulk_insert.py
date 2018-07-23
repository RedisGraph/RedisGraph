import redis
import csv
import os
import click

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

    def reset_tokens(self):
	for descriptor in self.descriptors:
	    descriptor.reset_tokens()

    def remove_descriptors(self, delete_count):
	for descriptor in self.descriptors[:delete_count]:
	    print 'Finished inserting "%s".' % descriptor.name
	del self.descriptors[:delete_count]

    def unroll(self):
	ret = [self.argtype, self.pending_inserts(), self.type_count()]
	for descriptor in self.descriptors:
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
	return 3 + sum(descriptor.token_count() for descriptor in self.descriptors)

    def type_count(self):
	return len(self.descriptors)

    def pending_inserts(self):
	return sum(desc.pending_inserts for desc in self.descriptors)

# The Descriptor class holds the name and element counts for an individual relationship or node label.
# After the contents of an Argument instance have been submitted to Redis in a GRAPH.BULK query,
# the contents of each descriptor contained in the query are unrolled.
# The `unroll` methods are unique to the data type.
class Descriptor:
    def __init__(self, csvfile):
	# A Label or Relationship name is set by the CSV file name
	self.name = os.path.splitext(os.path.basename(csvfile.name))[0]
	self.total_entities = 0
	self.entities_created = 0
	self.pending_inserts = 0
	self.reader = csv.reader(csvfile)

    def count_entities(self, csvfile):
	# Count number of lines in file
	self.total_entities = sum(1 for row in csvfile)
	# Reset iterator to start of file
	csvfile.seek(0)

    def print_progress(self):
	print '%.2f%% (%d / %d) of "%s" inserted.' % (float(self.entities_created) * 100 / self.total_entities,
		self.entities_created,
		self.total_entities,
		self.name)

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
	self.attributes = self.reader.next()
	self.attribute_count = len(self.attributes)

	# Count number of entities (line count - 1 for header row)
	self.count_entities(csvfile)

	# The CSV reader has been reset - skip the header row
	self.reader.next()

    def token_count(self):
	# Labels have a token for name, attribute count, pending insertion count, plus N tokens for the individual attributes.
	return 3 + self.attribute_count

    def unroll(self):
	return [self.name, self.pending_inserts, self.attribute_count] + self.attributes

# A RelationDescriptor consists of a name string and a relationship count.
# As a query string, a RelationDescriptor is printed in the format:
# [relation name, relation count]
class RelationDescriptor(Descriptor):
    def __init__(self, csvfile):
	Descriptor.__init__(self, csvfile)
	# Count number of entities (line count)
	self.count_entities(csvfile)

    def token_count(self):
	# Relations have 2 tokens: name and pending insertion count.
	return 2

    def unroll(self):
	return [self.name, self.pending_inserts]

max_tokens = 1024 * 1024
graphname = None
redis_client = None

def QueryRedis(metadata, entities):
    cmd = metadata.unroll() + entities
    # TODO - asynchronous execution of these queries would be really nice; we don't need to
    # wait for a response. These two approaches take about the same time, though -
    # find a solution

    #  pipe = redis_client.pipeline()
    #  result = pipe.execute_command("GRAPH.BULK", graphname, *cmd)
    #  pipe.execute()
    result = redis_client.execute_command("GRAPH.BULK",
	    graphname,
	    *cmd
	    )
    stats = result.split(', ')
    metadata.entities_created += int(stats[0].split(' ')[0])
    metadata.entities_created += int(stats[1].split(' ')[0])
    metadata.queries_processed += 1

def ProcessNodes(nodes_csv_files):
        labels = Argument("NODES")
	NODES = []		# List of nodes to be inserted
	# TODO This will not be a valid assumption when the script gains support for insertion of unlabeled nodes
	# Can use calls to the token_count methods, but that seems redundant
	TOKEN_COUNT = 5 # "GRAPH.BULK [graphname] LABELS [entity_count] [# of labels]"

	depleted_labels = 0
	# Process labeled nodes.
	for node_csv_file in nodes_csv_files:
	    with open(node_csv_file, 'rb') as csvfile:
		descriptor = LabelDescriptor(csvfile)
		print 'Inserting Label "%s" - %d nodes' % (descriptor.name, descriptor.total_entities)

		# 3 tokens for the descriptor name, insert count, and attribute count, and 1 for every attribute
		TOKEN_COUNT += 3 + descriptor.attribute_count
		labels.descriptors.append(descriptor)

		# Labeled nodes
		for row in descriptor.reader:
		    TOKEN_COUNT += len(row)
		    if TOKEN_COUNT > max_tokens:
			# max_tokens has been reached; submit all but the most recent node
			QueryRedis(labels, NODES)
			descriptor.entities_created += descriptor.pending_inserts
			# Reset values post-query
			labels.reset_tokens()
			NODES = []
			# If 1 or more CSVs have been depleted, remove their descriptors
			if depleted_labels > 0:
			    labels.remove_descriptors(depleted_labels)
			    depleted_labels = 0
			descriptor.print_progress()
			# Following an insertion, TOKEN_COUNT is set to accommodate all labels
			# and attributes, plus the individual tokens from the uninserted current row
			TOKEN_COUNT = 2 + labels.token_count() + len(row)
		    descriptor.pending_inserts += 1
		    NODES += row

		# A CSV has been fully processed; after the next insert its descriptor should be deleted
		depleted_labels += 1
	# Insert all remaining nodes
	QueryRedis(labels, NODES)
	labels.remove_descriptors(depleted_labels)
	print "%d Nodes created in %d queries." % (labels.entities_created, labels.queries_processed)

# TODO This might be sufficiently similar to ProcessNodes to allow for just one function with different descriptors
def ProcessRelations(relations_csv_files):
	# Introduce relations
	rels = Argument("RELATIONS")
	RELATIONS = []			    # Relations to be constructed
	TOKEN_COUNT = 5 # "GRAPH.BULK [graphname] RELATIONS [entity_count] [relation_count]

	depleted_relations = 0

	# Process relations.
	for relation_csv_file in relations_csv_files:
	    with open(relation_csv_file, 'rb') as csvfile:
		descriptor = RelationDescriptor(csvfile)
		print 'Inserting Relation "%s" - %d edges.' % (descriptor.name, descriptor.total_entities)

		# Increase counts to accommodate two additional relation tokens (name and count)
		TOKEN_COUNT += 2
		rels.descriptors.append(descriptor)

		for row in descriptor.reader:
		    TOKEN_COUNT += len(row)
		    if TOKEN_COUNT > max_tokens:
		    # max_tokens has been reached; submit all but the most recent entity
			QueryRedis(rels, RELATIONS)
			descriptor.entities_created += descriptor.pending_inserts
			# After the query, subtract submitted counts from rels
			rels.reset_tokens()
			RELATIONS = []
			# If 1 or more CSVs have been depleted, remove their descriptors
			if depleted_relations > 0:
				rels.remove_descriptors(depleted_relations)
				depleted_relations = 0
			descriptor.print_progress()
			# Following an insertion, TOKEN_COUNT is set to accommodate all relations
			# plus the individual tokens from the uninserted current row
			TOKEN_COUNT = 2 + rels.token_count() + len(row)
		    descriptor.pending_inserts += 1
		    RELATIONS += row

		# A CSV has been fully processed; after the next insert its descriptor should be deleted
		depleted_relations += 1

	# Insert all remaining relations
	QueryRedis(rels, RELATIONS)
	rels.remove_descriptors(depleted_relations)
	print "%d Relations created in %d queries." % (rels.entities_created, rels.queries_processed)

def help():
	pass

@click.command()
@click.argument('graph')
@click.option('--host', '-h', default='127.0.0.1', help='Redis server host')
@click.option('--port', '-p', default=6379, help='Redis server port')
@click.option('--nodes', '-n', multiple=True, help='path to node csv file')
@click.option('--relations', '-r', multiple=True, help='path to relation csv file')
@click.option('--max_buffer_size', '-m', default=1024*1024, help='max token count per Redis query')
def bulk_insert(graph, host, port, nodes, relations, max_buffer_size):
        global graphname
	global redis_client
	global max_tokens
        graphname = graph
	redis_client = redis.StrictRedis(host=host, port=port)
	if max_buffer_size > max_tokens:
	    print "Requested buffer size too large, capping queries at %d." % max_tokens
	else:
	    max_tokens = max_buffer_size

	if nodes:
	    ProcessNodes(nodes)

	if relations:
	    ProcessRelations(relations)

if __name__ == '__main__':
    bulk_insert()
