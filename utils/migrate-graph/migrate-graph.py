import re
import sys
import json
import redis
# from redis.commands.graph import Graph
# from redis.commands.graph.node import Node
# from redis.commands.graph.edge import Edge
import datetime
from RLTest import Env

################################################################################
# Auxiliary functions
################################################################################
def printTimeStamp():
    now = datetime.datetime.now()
    print ("Current date and time : ")
    print (now.strftime("%Y-%m-%d %H:%M:%S"))

def printTotalNodes(graph, graph_name):
    res = graph.query("MATCH (n) RETURN count(n)")
    print(graph_name, ' total nodes:', "{:10,}".format(res.result_set[0][0]))

def migrateNodes(src_graph, dest_graph, node_id_dict):
    # copy all nodes from src to dest
    batch_size = 25000

    res = src_graph.query("MATCH (n) RETURN max(id(n))")
    max_node_id = res.result_set[0][0]
    res = src_graph.query("MATCH (n) RETURN count(n)")
    total_nodes_src = res.result_set[0][0]
    print('SOURCE: total nodes:', "{:,}".format(total_nodes_src), 'max_node_id:', max_node_id)

    count = 0
    for src_node_id in range(0, max_node_id + 1):
        res = src_graph.query(f"MATCH (n) WHERE id(n) = {src_node_id}  RETURN n")
        if len(res.result_set) == 0:
            continue
        node1 = res.result_set[0][0]
        if node1.alias is None:
            node1.alias = 'Src_NodeId_' + str(src_node_id)
            # print(node1.alias)

        dest_graph.add_node(node1)
        node_id_dict[str(src_node_id)] = count
        count += 1
        if count % batch_size == 0:
            # print("{:10,}".format(count), 'nodes copied,', 'current_id', "{:10,}".format(src_node_id))
            dest_graph.flush()
            printTotalNodes(dest_graph, 'DEST')

    # print("{:10,}".format(count), 'nodes copied,', 'current_id', "{:10,}".format(src_node_id))
    # commit changes
    dest_graph.flush()
    printTotalNodes(dest_graph, 'DEST')

def migrateEdges(src_graph, dest_graph, node_id_dict):
    # copy all edges from src to dest
    batch_size = 5000
    res = src_graph.query("MATCH (n) RETURN max(id(n))")
    max_node_id = res.result_set[0][0]

    res = src_graph.query("MATCH ()-[r]->() RETURN max(id(r))")
    max_edge_id = res.result_set[0][0]

    res = src_graph.query("MATCH ()-[r]->() RETURN count(r)")
    total_edges_src = res.result_set[0][0]
    print('SOURCE: total edges:', "{:,}".format(total_edges_src), 'max_edge_id:', max_edge_id)

    count = 0
    for src_node_id in range(0, max_node_id + 1):
        res = src_graph.query(f"MATCH (n)-[r]->(m) WHERE id(n) = {src_node_id} RETURN r, id(n), id(m)")
        if len(res.result_set) == 0:
            continue
        
        for row in res.result_set:
            src_graph_Edge = row[0]
            id_n = row[1]
            id_m = row[2]

            dest_graph_src_nodeId = node_id_dict.get(str(id_n), -1)
            dest_graph_dest_nodeId = node_id_dict.get(str(id_m), -1)
            if dest_graph_src_nodeId == -1:
                sys.exit(f"Unknown source node id in edge: {id_n}. Exiting...")
            if dest_graph_dest_nodeId == -1:
                sys.exit(f"Unknown dest node id in edge: {id_m}. Exiting...")

            # Convert to string
            properties_string = json.dumps(src_graph_Edge.properties)
            # Remove quotes from keys
            properties_string = re.sub(r'\"(\w+)\":', r'\1:', properties_string)
            query = f"MATCH (n) WHERE id(n)= {dest_graph_src_nodeId} WITH n MATCH (m) WHERE id(m)= {dest_graph_dest_nodeId} WITH n,m CREATE (n)-[:{src_graph_Edge.relation} {properties_string}]->(m)"
            dest_graph.query(query)
            count += 1
            if count % batch_size == 0:
                print("{:10,}".format(count), 'edges copied,', 'current_node_id:', "{:10}".format(src_node_id))
                dest_graph.flush()

    print("{:10,}".format(count), 'edges copied,', 'current_node_id', "{:10}".format(src_node_id))
    # commit changes
    dest_graph.flush()

def validateGraphs(src_graph, dest_graph, node_id_dict):
    # check if the number of nodes and edges are the same in both graphs
    print('Validating...')
    # print source graph stats
    res = src_graph.query("MATCH (n) RETURN max(id(n))")
    max_id = res.result_set[0][0]
    res = src_graph.query("MATCH (n) RETURN count(distinct id(n))")
    src_total_nodes = res.result_set[0][0]
    print('SOURCE: total nodes:', "{:,}".format(src_total_nodes), 'max_id:', max_id)

    res = src_graph.query("MATCH (n)-[r]->(m) RETURN max(id(r))")
    dest_max_id = res.result_set[0][0]
    res = src_graph.query("MATCH (n)-[r]->(m) RETURN count(distinct id(r))")
    src_total_edges = res.result_set[0][0]
    print('SOURCE: total edges:', "{:,}".format(src_total_edges), 'max_id:', dest_max_id)

    # print destination graph stats
    res = dest_graph.query("MATCH (n) RETURN max(id(n))")
    max_id = res.result_set[0][0]
    res = dest_graph.query("MATCH (n) RETURN count(distinct id(n))")
    dest_total_nodes = res.result_set[0][0]
    print('DEST: total nodes:', "{:,}".format(dest_total_nodes), 'max_id:', max_id)

    res = dest_graph.query("MATCH (n)-[r]->(m) RETURN max(id(r))")
    dest_max_id = res.result_set[0][0]
    res = dest_graph.query("MATCH (n)-[r]->(m) RETURN count(distinct id(r))")
    dest_total_edges = res.result_set[0][0]
    print('DEST: total edges:', "{:,}".format(dest_total_edges), 'max_id:', dest_max_id)

    if src_total_nodes != dest_total_nodes:
        sys.exit('Error: Total nodes in source and destination graphs do not match. Exiting...')
    if src_total_edges != dest_total_edges:
        sys.exit('Error: Total edges in source and destination graphs do not match. Exiting...')

    # check if the nodes are the same in both graphs
    for src_node_id in range(0, src_total_nodes):
        res_src = src_graph.query(f"MATCH (n) WHERE id(n) = {src_node_id} RETURN n")
        res_dest = dest_graph.query(f"MATCH (n) WHERE id(n) = {node_id_dict[str(src_node_id)]} RETURN n")
        if len(res_src.result_set) == 0 or len(res_dest.result_set) == 0:
            sys.exit(f'Error: Node with id {src_node_id} not found in source or destination graph. Exiting...')
        env = Env(decodeResponses=True)
        env.assertEqual(res_src.result_set, res_dest.result_set)

    # TODO: check if the edges are the same in both graphs
    # check if the edges are the same in both graphs

    
    print('Graphs validated successfully!')

################################################################################
# Main
################################################################################

src = redis.Redis(host='localhost', port=6379, decode_responses=True)
dest = redis.Redis(host='localhost', port=6380, decode_responses=True)

src_graph = src.graph('Merchant')
dest_graph = dest.graph('Merchant')
node_id_dict = {}

createNodes = True
createEdges = True
validate = True

if createNodes:
    migrateNodes(src_graph, dest_graph, node_id_dict)
    with open('node_id_dict.json', 'w') as f:
        json.dump(node_id_dict, f)
else:
    # Read from file
    with open('node_id_dict.json', 'r') as f:
        node_id_dict = json.load(f)

if createEdges:
    migrateEdges(src_graph, dest_graph, node_id_dict)

if validate:
    # validate graphs
    validateGraphs(src_graph, dest_graph, node_id_dict)


