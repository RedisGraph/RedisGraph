import redis
import csv
import os
import click
# TODO for debugging only; remove
import ipdb

class Argument:
    def __init__(self, argtype):
	self.argtype = argtype
	self.pending_inserts = 0
	self.type_count = 0
	self.descriptors = []

    def reset_tokens(self):
	self.pending_inserts = 0
        for descriptor in self.descriptors:
	    descriptor.pending_inserts = 0

    def remove_descriptors(self, delete_count):
	del self.descriptors[:delete_count]
	self.type_count -= delete_count

    def unroll(self):
	if self.pending_inserts <= 0:
	    print "No pending inserts on Argument?"
	ret = [self.argtype, self.pending_inserts, self.type_count]
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

# The structure is:
# structure of nodeinsert:
# ["NODES", node count, label count, label_descriptors[0..n]]
# structure of a label descriptor:
# [label name, node count, attribute count, attributes[0..n]]
class Descriptor:
    def __init__(self, name):
	self.name = name
	self.pending_inserts = 0


class LabelDescriptor(Descriptor):
    def __init__(self, name):
	self.name = name
	self.pending_inserts = 0
	self.attribute_count = 0
	self.attributes = []

    def token_count(self):
	# Labels have a token for name, attribute count, pending insertion count, plus N tokens for the individual attributes.
	return 3 + self.attribute_count

    def unroll(self):
	return [self.name, self.pending_inserts, self.attribute_count] + self.attributes

# TODO This seems kind of unnecessary, and should maybe be refactored
class RelationDescriptor(Descriptor):
    def token_count(self):
	# Relations have 2 tokens: name and pending insertion count.
	return 2

    def unroll(self):
	return [self.name, self.pending_inserts]

MAX_TOKENS = 1024 * 1024
#  MAX_TOKENS = 8
graphname = None
redis_client = None

def QueryRedis(metadata, entities):
    #  pipe = redis_client.pipeline()
    #  result = pipe.execute_command("GRAPH.BULK", graph, *ARGS)
    #  pipe.execute()
    cmd = metadata.unroll() + entities
    result = redis_client.execute_command("GRAPH.BULK",
	    graphname,
	    *cmd
	    )

def ProcessNodes(nodes_csv_files):
        labels = Argument("NODES")
	NODES = []		# List of nodes to be inserted
	# TODO This will not be a valid assumption when the script gains support for insertion of unlabeled nodes
	# Can use calls to the token_count methods, but that seems redundant
	TOKEN_COUNT = 5 # "GRAPH.BULK [graphname] LABELS [entity_count] [type_count]"

	depleted_labels = 0
	# Process labeled nodes.
	for node_csv_file in nodes_csv_files:
		with open(node_csv_file, 'rb') as csvfile:
			reader = csv.reader(csvfile)
			header_row = reader.next()

			# Make a new descriptor for this label
			label_name = os.path.splitext(os.path.basename(node_csv_file))[0]
			descriptor = LabelDescriptor(label_name)
			# The first row of a label CSV contains its attributes
			descriptor.attributes = header_row

			# Update counts
			header_len = len(header_row)
			descriptor.attribute_count += header_len
			# 3 tokens for the descriptor name, insert count, and attribute count, and 1 for every attribute
			TOKEN_COUNT += 3 + header_len

			# New label - process header row and add to `labels`
			labels.descriptors.append(descriptor)
			labels.type_count += 1

			# Labeled nodes
			for row in reader:
				TOKEN_COUNT += len(row)
				if TOKEN_COUNT > MAX_TOKENS:
				    # MAX_TOKENS has been reached; submit all but the most recent node
				    QueryRedis(labels, NODES)
				    # Reset values post-query
				    labels.reset_tokens()
				    # Reset token count, including the one remaining row
				    TOKEN_COUNT = 2 + labels.token_count() + len(row)
				    NODES = []
				    # If 1 or more CSVs have been depleted, remove their descriptors
				    if depleted_labels > 0:
					labels.remove_descriptors(depleted_labels)
					depleted_labels = 0
				labels.pending_inserts += 1
				descriptor.pending_inserts += 1
				NODES += row

			# A CSV has been fully processed; after the next insert its descriptor should be deleted
			depleted_labels += 1
	# Insert all remaining nodes
	QueryRedis(labels, NODES)

# TODO This might be sufficiently similar to ProcessNodes to allow for just one function with different descriptors
def ProcessRelations(relations_csv_files):
	# Introduce relations
	rels = Argument("RELATIONS")
	RELATIONS = []			    # Relations to be constructed
	TOKEN_COUNT = 2 # "GRAPH.BULK [graphname]

	depleted_relations = 0

	# Process relations.
	for relation_csv_file in relations_csv_files:
		with open(relation_csv_file, 'rb') as csvfile:
			reader = csv.reader(csvfile)

			# Make a new descriptor for this relation
			relation_name = os.path.splitext(os.path.basename(relation_csv_file))[0]
			descriptor = RelationDescriptor(relation_name)

			rels.descriptors.append(descriptor)
			# Increase counts to accommodate two additional relation tokens (name and count)
			TOKEN_COUNT += 2
			rels.type_count += 1

			for row in reader:
				TOKEN_COUNT += len(row)
				if TOKEN_COUNT > MAX_TOKENS:
				    # MAX_TOKENS has been reached; submit all but the most recent entity
				    QueryRedis(rels, RELATIONS)
				    # After the query, subtract submitted counts from rels
				    rels.reset_tokens()
				    # Reset token count, including the one remaining row
				    TOKEN_COUNT = 2 + rels.token_count() + len(row)
				    RELATIONS = []
				    # If 1 or more CSVs have been depleted, remove their descriptors
				    if depleted_relations > 0:
					rels.remove_descriptors(depleted_relations)
					depleted_relations = 0
				rels.pending_inserts += 1
				descriptor.pending_inserts += 1
				RELATIONS += row

			# A CSV has been fully processed; after the next insert its descriptor should be deleted
			depleted_relations += 1

	# Insert all remaining relations
	QueryRedis(rels, RELATIONS)

def help():
	pass

@click.command()
@click.argument('graph')
@click.option('--host', '-h', default='127.0.0.1', help='Redis server host')
@click.option('--port', '-p', default=6379, help='Redis server port')
@click.option('--nodes', '-n', multiple=True, help='path to node csv file')
@click.option('--relations', '-r', multiple=True, help='path to relation csv file')
def bulk_insert(graph, host, port, nodes, relations):
        global graphname
	global redis_client
        graphname = graph
	redis_client = redis.StrictRedis(host=host, port=port)

	if nodes:
	    ProcessNodes(nodes)

	if relations:
	    ProcessRelations(relations)

if __name__ == '__main__':
    bulk_insert()
