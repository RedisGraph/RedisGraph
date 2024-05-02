import redis
import datetime

################################################################################
# Auxiliary functions
################################################################################

def printTimeStamp(begin=True):
    """Prints the current date and time."""

    now = datetime.datetime.now()
    print("Date and time", "of test START" if begin else "of test END", f": {now.strftime('%Y-%m-%d %H:%M:%S')}")

def printGraphData(graph):
    """Prints graph node and edge count, and returns that maximum node-id in the
    graph."""

    res = graph.query("MATCH (n) RETURN max(id(n)), count(n)", read_only=True)
    max_node_id = res.result_set[0][0]
    total_nodes_src = res.result_set[0][1]
    print('Total nodes:', "{:,}".format(total_nodes_src), 'max_node_id:', "{:,}".format(max_node_id))

    res = graph.query("MATCH ()-[r]->() RETURN max(id(r)), count(r)", read_only=True)
    max_edge_id = res.result_set[0][0]
    total_edges_src = res.result_set[0][1]
    print('Total edges:', "{:,}".format(total_edges_src), 'max_edge_id:', "{:,}".format(max_edge_id))

    return max_node_id

def checkSrcAndDestNodes(src_graph):
    print(f'Searching for corrupted entities in Graph: {src_graph.NAME}')
    printTimeStamp()
    max_node_id = printGraphData(src_graph)

    batch_size = 10000
    errors = []

    # Queries
    q1 = "MATCH (n)-[r]->(m) WHERE id(n) = $id_n RETURN id(m), id(r)"
    q2 = "MATCH (n)<-[r]-(m) WHERE id(n) = $id_n RETURN id(m), id(r)"

    # for node_id in range(0, max_node_id + 1):
    for node_id in range(max_node_id + 1):
        for q in [q1, q2]:
            results = src_graph.query(f"CYPHER id_n={node_id} {q}", read_only=True).result_set
            if len(results) == 0:
                # no edges for this node.
                continue
            
            for row in results:
                id_n = node_id
                id_m = row[0]
                id_r = row[1]
                # check other node
                res = src_graph.query(f"CYPHER id_m = {id_m} MATCH (m) WHERE id(m) = $id_m RETURN 1", read_only=True)
                if len(res.result_set) == 0:
                    err_msg = 'Error in path: ' + (f'(n)-[r]->(m):' if q is q1 else f'(n)<-[r]-(m):') + f' id(n)={id_n} id(r)={id_r}, id(m)={id_m}: Node with id {id_m} not found.'
                    errors.append(err_msg)

        if node_id % batch_size == 0:
            print(f'Traversed {node_id} node ids...')

    if node_id % batch_size != 0:
        print(f'Traversed {node_id} node ids...')

    print('Traversal finished!')
    printTimeStamp(False)

    print(f'Encountered {len(errors)} errors overall{":" if len(errors) > 0 else ""}')
    for e in errors:
        print(e)

################################################################################
# Main
################################################################################

HOST = "localhost"
PORT = 6379
# HOST = "redis-18763.raz-mon-aws-cluster-kabmc.env0.qa.redislabs.com"
# PORT = 18763

src = redis.Redis(host=HOST, port=PORT, decode_responses=True)
graph = src.graph('Merchant')

if __name__ == "__main__":
    # Search for corrupted edges in the given graph.
    checkSrcAndDestNodes(graph)
