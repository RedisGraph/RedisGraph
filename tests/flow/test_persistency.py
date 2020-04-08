from base import FlowTestsBase
import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))


redis_con = None
GRAPH_NAME = "G"
DENSE_GRAPH_NAME = "H"
redis_graph = None
dense_graph = None
people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]
countries = ["Israel", "USA", "Japan", "United Kingdom"]
visits = [("Roi", "USA"), ("Alon", "Israel"),
          ("Ailon", "Japan"), ("Boaz", "United Kingdom")]


class testGraphPersistency(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        global dense_graph
        global redis_con
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_NAME, redis_con)
        dense_graph = Graph(DENSE_GRAPH_NAME, redis_con)
        self.populate_graph()
        self.populate_dense_graph()

    def populate_graph(self):
        global redis_graph

        if not redis_con.exists(GRAPH_NAME):
            personNodes = {}
            countryNodes = {}
            # Create entities

            for p in people:
                person = Node(label="person", properties={"name": p})
                redis_graph.add_node(person)
                personNodes[p] = person

            for p in countries:
                country = Node(label="country", properties={"name": p})
                redis_graph.add_node(country)
                countryNodes[p] = country

            for v in visits:
                person = v[0]
                country = v[1]
                edge = Edge(personNodes[person], 'visit', countryNodes[country], properties={
                            'purpose': 'pleasure'})
                redis_graph.add_edge(edge)

            redis_graph.commit()

            # Delete nodes, to introduce deleted item within our datablock
            query = """MATCH (n:person) WHERE n.name = 'Roi' or n.name = 'Ailon' DELETE n"""
            redis_graph.query(query)

            query = """MATCH (n:country) WHERE n.name = 'USA' DELETE n"""
            redis_graph.query(query)

            # Create index.
            actual_result = redis_con.execute_command(
                "GRAPH.QUERY", GRAPH_NAME, "CREATE INDEX ON :person(name)")
            actual_result = redis_con.execute_command(
                "GRAPH.QUERY", GRAPH_NAME, "CREATE INDEX ON :country(name)")

    def populate_dense_graph(self):
        global dense_graph

        if not redis_con.exists(DENSE_GRAPH_NAME):
            nodes = []
            for i in range(10):
                node = Node(label="n", properties={"val": i})
                dense_graph.add_node(node)
                nodes.append(node)

            for n_idx, n in enumerate(nodes):
                for m_idx, m in enumerate(nodes[:n_idx]):
                    dense_graph.add_edge(Edge(n, "connected", m))

            dense_graph.commit()

    #  Connect a single node to all other nodes.
    def test01_save_load_rdb(self):
        for i in range(2):
            if i == 1:
                # Save RDB & Load from RDB
                self.env.dumpAndReload()

            # Verify
            # Expecting 5 person entities.
            query = """MATCH (p:person) RETURN COUNT(p)"""
            actual_result = redis_graph.query(query)
            nodeCount = actual_result.result_set[0][0]
            self.env.assertEquals(nodeCount, 5)

            query = """MATCH (p:person) WHERE p.name='Alon' RETURN COUNT(p)"""
            actual_result = redis_graph.query(query)
            nodeCount = actual_result.result_set[0][0]
            self.env.assertEquals(nodeCount, 1)

            # Expecting 3 country entities.
            query = """MATCH (c:country) RETURN COUNT(c)"""
            actual_result = redis_graph.query(query)
            nodeCount = actual_result.result_set[0][0]
            self.env.assertEquals(nodeCount, 3)

            query = """MATCH (c:country) WHERE c.name = 'Israel' RETURN COUNT(c)"""
            actual_result = redis_graph.query(query)
            nodeCount = actual_result.result_set[0][0]
            self.env.assertEquals(nodeCount, 1)

            # Expecting 2 visit edges.
            query = """MATCH (n:person)-[e:visit]->(c:country) WHERE e.purpose='pleasure' RETURN COUNT(e)"""
            actual_result = redis_graph.query(query)
            edgeCount = actual_result.result_set[0][0]
            self.env.assertEquals(edgeCount, 2)

            # Verify indices exists.
            plan = redis_graph.execution_plan(
                "MATCH (n:person) WHERE n.name = 'Roi' RETURN n")
            self.env.assertIn("Index Scan", plan)

            plan = redis_graph.execution_plan(
                "MATCH (n:country) WHERE n.name = 'Israel' RETURN n")
            self.env.assertIn("Index Scan", plan)

    # Verify that edges are not modified after entity deletion
    def test02_deleted_entity_migration(self):
        query = """MATCH (p) WHERE ID(p) = 0 OR ID(p) = 3 OR ID(p) = 7 OR ID(p) = 9 DELETE p"""
        actual_result = dense_graph.query(query)
        self.env.assertEquals(actual_result.nodes_deleted, 4)
        query = """MATCH (p)-[]->(q) RETURN p.val, q.val ORDER BY p.val, q.val"""
        first_result = dense_graph.query(query)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        second_result = dense_graph.query(query)
        self.env.assertEquals(first_result.result_set,
                              second_result.result_set)

    # Strings, numerics, booleans, arrays and NULL properties should be properly serialized and reloaded
    def test03_restore_properties(self):
        graphname = "simple_props"
        graph = Graph(graphname, redis_con)
        query = """CREATE (:p {strval: 'str', numval: 5.5, nullval: NULL, boolval: true, array: [1,2,3]})"""
        actual_result = graph.query(query)
        # Verify that node was created correctly
        self.env.assertEquals(actual_result.nodes_created, 1)
        self.env.assertEquals(actual_result.properties_set, 5)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        query = """MATCH (p) RETURN p.boolval, p.nullval, p.numval, p.strval, p.array"""
        actual_result = graph.query(query)

        # Verify that the properties are loaded correctly.
        # Note that the order of results is not guaranteed (currently managed by the Schema),
        # so this may need to be updated in the future.
        expected_result = [[True, None, 5.5, 'str', [1, 2, 3]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Verify multiple edges of the same relation between nodes A and B
    # are saved and restored correctly.
    def test04_repeated_edges(self):
        graphname = "repeated_edges"
        g = Graph(graphname, redis_con)
        src = Node(label='p', properties={'name': 'src'})
        dest = Node(label='p', properties={'name': 'dest'})
        edge1 = Edge(src, 'e', dest, properties={'val': 1})
        edge2 = Edge(src, 'e', dest, properties={'val': 2})
        g.add_node(src)
        g.add_node(dest)
        g.add_edge(edge1)
        g.add_edge(edge2)
        g.flush()

        # Verify the new edge
        q = """MATCH (a)-[e]->(b) RETURN e.val, a.name, b.name ORDER BY e.val"""
        actual_result = g.query(q)

        expected_result = [[edge1.properties['val'], src.properties['name'], dest.properties['name']],
                           [edge2.properties['val'], src.properties['name'], dest.properties['name']]]

        self.env.assertEquals(actual_result.result_set, expected_result)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        # Verify that the latest edge was properly saved and loaded
        actual_result = g.query(q)
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test05_tagged_graph(self):
        graphname = "replication_test_{tagged_graph}"
        graph = Graph(graphname, redis_con)
        query = """CREATE (:p {strval: 'str', numval: 5.5, nullval: NULL, boolval: true, array: [1,2,3]})"""
        actual_result = graph.query(query)
        # Verify that node was created correctly
        self.env.assertEquals(actual_result.nodes_created, 1)
        self.env.assertEquals(actual_result.properties_set, 5)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        query = """MATCH (p) RETURN p.boolval, p.nullval, p.numval, p.strval, p.array"""
        actual_result = graph.query(query)

        # Verify that the properties are loaded correctly.
        # Note that the order of results is not guaranteed (currently managed by the Schema),
        # so this may need to be updated in the future.
        expected_result = [[True, None, 5.5, 'str', [1, 2, 3]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        src = Node(label='p', properties={'name': 'src'})
        dest = Node(label='p', properties={'name': 'dest'})
        edge1 = Edge(src, 'e', dest, properties={'val': 1})
        edge2 = Edge(src, 'e', dest, properties={'val': 2})
        graph.add_node(src)
        graph.add_node(dest)
        graph.add_edge(edge1)
        graph.add_edge(edge2)
        graph.flush()

        # Verify the new edge
        q = """MATCH (a)-[e]->(b) RETURN e.val, a.name, b.name ORDER BY e.val"""
        actual_result = graph.query(q)

        expected_result = [[edge1.properties['val'], src.properties['name'], dest.properties['name']],
                           [edge2.properties['val'], src.properties['name'], dest.properties['name']]]

        self.env.assertEquals(actual_result.result_set, expected_result)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        # Verify that the latest edge was properly saved and loaded
        actual_result = graph.query(q)
        self.env.assertEquals(actual_result.result_set, expected_result)
