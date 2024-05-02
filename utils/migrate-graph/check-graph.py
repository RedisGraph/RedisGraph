import re
import sys
import json
import redis
# from redis.commands.graph import Graph
# from redis.commands.graph.node import Node
# from redis.commands.graph.edge import Edge
import hashlib
import datetime
from RLTest import Env

################################################################################
# Auxiliary functions
################################################################################
def printTimeStamp():
    now = datetime.datetime.now()
    print ("Current date and time : ", now.strftime("%Y-%m-%d %H:%M:%S"))

def printTotalNodes(graph, graph_name):
    res = graph.query("MATCH (n) RETURN count(n)", read_only=True)
    print(graph_name, ' total nodes:', "{:10,}".format(res.result_set[0][0]))

def checkDestNodes(src_graph):
    printTimeStamp()
    batch_size = 50000
    res = src_graph.query("MATCH (n) RETURN max(id(n))", read_only=True)
    max_node_id = res.result_set[0][0]

    res = src_graph.query("MATCH ()-[r]->() RETURN max(id(r))", read_only=True)
    max_edge_id = res.result_set[0][0]

    res = src_graph.query("MATCH ()-[r]->() RETURN count(r)", read_only=True)
    total_edges_src = res.result_set[0][0]
    print('SOURCE: total edges:', "{:,}".format(total_edges_src), 'max_edge_id:', max_edge_id)

    src_node_count = 0
    dest_node_count = 0
    total_validations = 0
    errors = 0
    for src_node_id in range(0, max_node_id + 1):
        res = src_graph.query(f"CYPHER id_n={src_node_id} MATCH (n)-[r1]->(m) WHERE id(n) = $id_n WITH n, r1, m MATCH (n)<-[r2]-(p) RETURN id(m), id(r1), id(p), id(r2)", read_only=True)
        if len(res.result_set) == 0:
            continue # no edges for this node. Is this an error?
        
        for row in res.result_set:
            id_n = src_node_id
            id_m = row[0]
            id_r1 = row[1]
            id_p = row[2]
            id_r2 = row[3]
            # check dest node
            res = src_graph.query(f"CYPHER id_m = {id_m} MATCH (m) WHERE id(m) = $id_m RETURN 1", read_only=True)
            if len(res.result_set) == 0:
                errors += 1
                print()
                print(f'Error in path: (n)-[r1]->(m): id(n)={id_n} id(r1)={id_r1}, id(m)={id_m}')
                print(f'Node with id {id_m} not found.')
            dest_node_count += 1
            total_validations += 1
            # Check source node
            res = src_graph.query(f"CYPHER id_p = {id_p} MATCH (p) WHERE id(p) = $id_p RETURN 1", read_only=True)
            if len(res.result_set) == 0:
                errors += 1
                print()
                print(f'Error in path: (n)<-[r2]-(p): id(n)={id_n} id(r2)={id_r2}, id(m)={id_p}')
                print(f'Node with id {id_p} not found.')
            src_node_count += 1
            total_validations += 1
            if total_validations % batch_size == 0:
                print("{:10,}".format(total_validations), 'validations done,', 'current_id', "{:10,}".format(src_node_id))

    print("{:10,}".format(total_validations), 'validations done,', 'current_id', "{:10,}".format(src_node_id))
    print('Destination and Source nodes validation finished')
    print('Errors:', errors)
    printTimeStamp()

################################################################################
# Main
################################################################################

src = redis.Redis(host='localhost', port=6379, decode_responses=True)

src_graph = src.graph('Merchant')

checkDestNodes(src_graph)
