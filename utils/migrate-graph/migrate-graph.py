import redis
# from redis.commands.graph import Graph
from redis.commands.graph.node import Node
from redis.commands.graph.edge import Edge

src = redis.Redis(host='localhost', port=6379, decode_responses=True)
dest = redis.Redis(host='localhost', port=6380, decode_responses=True)

src_graph = src.graph('GRAPH_NAME')
dest_graph = dest.graph('GRAPH_NAME')

batch_size = 25000

# copy all nodes from src to dest
res = src_graph.query("MATCH (n) RETURN max(id(n))")
max_id = res.result_set[0][0]
res = src_graph.query("MATCH (n) RETURN count(n)")
total_nodes_src = res.result_set[0][0]
print('SOURCE: total nodes:', "{:,}".format(total_nodes_src), 'max_id:', max_id)

count = 0
for i in range(0, max_id + 1):
    res = src_graph.query(f"MATCH (n) WHERE id(n) = {i}  RETURN n")
    if len(res.result_set) == 0:
        continue
    node1 = res.result_set[0][0]
    if node1.alias is None:
        node1.alias = 'Src_NodeId_' + str(i)
        # print(node1.alias)

    dest_graph.add_node(node1)
    count += 1
    if count % batch_size == 0:
        print("{:10,}".format(count), 'nodes copied,', 'current_id', "{:10,}".format(i))
        # res = dest_graph.query("MATCH (n) RETURN count(n)")
        # total_nodes_dest = res.result_set[0][0]
        # print('DEST: total nodes:', total_nodes_dest)
        # (Raz) Why not commit here? The nodes aren't really copied to the graph until the commit is called.
        # dest_graph.commit()

print("{:10,}".format(count), 'nodes copied,', 'current_id', "{:10,}".format(i))

# copy all edges from src to dest
batch_size = 5000
res = src_graph.query("MATCH ()-[r]->() RETURN max(id(r))")
max_id = res.result_set[0][0]
res = src_graph.query("MATCH ()-[r]->() RETURN count(r)")
total_edges_src = res.result_set[0][0]
print('SOURCE: total edges:', "{:,}".format(total_edges_src), 'max_id:', max_id)

count = 0
for i in range(0, max_id+1):
    res = src_graph.query(f"MATCH (n)-[r]->(m) WHERE id(r) = {i} RETURN r, id(n), id(m)")
    print(".", end='')
    if len(res.result_set) == 0:
        continue
    src_graph_Edge = res.result_set[0][0]
    id_n = res.result_set[0][1]
    id_m = res.result_set[0][2]

    dest_graph_srcNode = dest_graph.nodes['Src_NodeId_' + str(id_n)]
    print("^", end='')
    dest_graph_dstNode = dest_graph.nodes['Src_NodeId_' + str(id_m)]
    print(".", end='')

    dest_graph_Edge = Edge(dest_graph_srcNode, src_graph_Edge.relation, dest_graph_dstNode, src_graph_Edge.properties)
    dest_graph.add_edge(dest_graph_Edge)
    count += 1
    if count % batch_size == 0:
        print()
        print("{:10,}".format(count), 'edges copied,', 'current_id:', "{:10}".format(i))
        # dest_graph.commit()

print("{:10,}".format(count), 'edges copied,', 'current_id', "{:10}".format(i))
# commit changes
dest_graph.commit()

# --------------------------------- Validation ---------------------------------
print('Validating...')
# check if the number of nodes and edges are the same in both graphs
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

# TODO: Add a validation of the entities themselves (node_i_src == node_i_dest, edge_i_src == edge_i_dest)
