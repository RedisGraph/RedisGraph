import redis
import csv
import os
import click

def ProcessNodes(LABELS, NODES, nodes_csv_files):
	# Introduce nodes
	LABELS.append("NODES")
	NODE_COUNT_IDX = len(LABELS)
	NODES_TO_CREATE = 0
	LABELS.append(NODES_TO_CREATE)

	# Number of labels
	LABELS.append(len(nodes_csv_files))

	# Process labeled nodes.
	for node_csv_file in nodes_csv_files:
		with open(node_csv_file, 'rb') as csvfile:
			reader = csv.reader(csvfile)
			header_row = reader.next()

			# Process header row.
			label_name = os.path.splitext(os.path.basename(node_csv_file))[0]
			LABELS.append(label_name)

			LABELED_NODE_COUNT = 0
			LABELED_NODE_COUNT_IDX = len(LABELS)
			LABELS.append(LABELED_NODE_COUNT)

			# Label attributes
			LABELS.append(len(header_row))
			LABELS += header_row

			# Labeled nodes
			for row in reader:
				LABELED_NODE_COUNT += 1
				NODES += row

			LABELS[LABELED_NODE_COUNT_IDX] = LABELED_NODE_COUNT
			NODES_TO_CREATE += LABELED_NODE_COUNT

	# Update total number of nodes to create.
	LABELS[NODE_COUNT_IDX] = NODES_TO_CREATE

def ProcessRelations(RELATION_METADATA, RELATIONS, relations_csv_files):
	# Introduce relations
	RELATION_METADATA.append("RELATIONS")
	RELATIONS_COUNT = 0
	RELATIONS_COUNT_IDX = len(RELATION_METADATA)
	RELATION_METADATA.append(RELATIONS_COUNT)
	RELATION_METADATA.append(len(relations_csv_files)) # Number of different relation types.

	# Process relations.
	for relation_csv_file in relations_csv_files:
		with open(relation_csv_file, 'rb') as csvfile:
			reader = csv.reader(csvfile)

			relation_name = os.path.splitext(os.path.basename(relation_csv_file))[0]
			RELATION_METADATA.append(relation_name)
			RELATION_COUNT = 0
			RELATION_COUNT_IDX = len(RELATION_METADATA)
			RELATION_METADATA.append(RELATION_COUNT)

			for row in reader:
				RELATION_COUNT += 1
				RELATIONS += row
			
			RELATION_METADATA[RELATION_COUNT_IDX] = RELATION_COUNT
			RELATIONS_COUNT += RELATION_COUNT

	# Update total number of relations.
	RELATION_METADATA[RELATIONS_COUNT_IDX] = RELATIONS_COUNT

def help():
	pass

@click.command()
@click.argument('graph')
@click.option('--nodes', multiple=True, help='path to node csv file')
@click.option('--relations', multiple=True, help='path to relation csv file')
def bulk_insert(graph, nodes, relations):
	ARGS = []		# Arguments for bulk insert command
	LABELS = [] 	# Labels information
	NODES = []		# Nodes data
	RELATIONS = []	# Relations data
	RELATION_METADATA = []	

	if nodes:
		ProcessNodes(LABELS, NODES, nodes)
		ARGS += LABELS
		ARGS += NODES

	if relations:
		ProcessRelations(RELATION_METADATA, RELATIONS, relations)
		ARGS += RELATION_METADATA
		ARGS += RELATIONS
	
	if ARGS:
		print ARGS
		redis_client = redis.StrictRedis()
		result = redis_client.execute_command("GRAPH.BULK", graph, *ARGS)
		print result

if __name__ == '__main__':
    bulk_insert()
