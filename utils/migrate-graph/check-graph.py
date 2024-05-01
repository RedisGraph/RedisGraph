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
    res = graph.query("MATCH (n) RETURN count(n)")
    print(graph_name, ' total nodes:', "{:10,}".format(res.result_set[0][0]))

def checkDestNodes(src_graph):
    printTimeStamp()
    batch_size = 50000
    res = src_graph.query("MATCH (n) RETURN max(id(n))")
    max_node_id = res.result_set[0][0]

    res = src_graph.query("MATCH ()-[r]->() RETURN max(id(r))")
    max_edge_id = res.result_set[0][0]

    res = src_graph.query("MATCH ()-[r]->() RETURN count(r)")
    total_edges_src = res.result_set[0][0]
    print('SOURCE: total edges:', "{:,}".format(total_edges_src), 'max_edge_id:', max_edge_id)

    count = 0
    errors = 0
    for src_node_id in range(0, max_node_id + 1):
        res = src_graph.query(f"MATCH (n)-[r]->(m) WHERE id(n) = {src_node_id} RETURN id(m), id(n), id(r)")
        if len(res.result_set) == 0:
            continue # no edges for this node. Is this an error?
        
        for row in res.result_set:
            id_m = row[0]
            id_n = row[1]
            id_r = row[2]
            res = src_graph.query(f"MATCH (n) WHERE id(n) = {id_m} RETURN 1")
            if len(res.result_set) == 0:
                errors += 1
                print()
                print(f'Error in path: (n)-[r]->(m): id(n)={id_n} id(r)={id_r}, id(m)={id_m}')
                print(f'Node with id {id_m} not found.')
            count += 1
            if count % batch_size == 0:
                print("{:10,}".format(count), 'edges checked,', 'current_id', "{:10,}".format(src_node_id))

    print("{:10,}".format(count), 'edges checked,', 'current_id', "{:10,}".format(src_node_id))
    print('Dest nodes validation finished')
    print('Errors:', errors)
    printTimeStamp()

################################################################################
# Main
################################################################################

src = redis.Redis(host='localhost', port=6379, decode_responses=True)

src_graph = src.graph('Merchant')

checkDestNodes(src_graph)
