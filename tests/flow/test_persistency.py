from base import FlowTestsBase
import os
import sys
import random
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))


redis_con = None

class testGraphPersistency(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_con
        redis_con = self.env.getConnection()

    def populate_graph(self, graph_name):
        # quick return if graph already exists
        if redis_con.exists(graph_name):
            return redis_graph

        people       = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]
        visits       = [("Roi", "USA"), ("Alon", "Israel"), ("Ailon", "Japan"), ("Boaz", "United Kingdom")]
        countries    = ["Israel", "USA", "Japan", "United Kingdom"]
        redis_graph  = Graph(graph_name, redis_con)
        personNodes  = {}
        countryNodes = {}

        # create nodes
        for p in people:
            person = Node(label="person", properties={"name": p, "height": random.randint(160, 200)})
            redis_graph.add_node(person)
            personNodes[p] = person

        for p in countries:
            country = Node(label="country", properties={"name": p, "population": random.randint(100, 400)})
            redis_graph.add_node(country)
            countryNodes[p] = country

        # create edges
        for v in visits:
            person  = v[0]
            country = v[1]
            edge = Edge(personNodes[person], 'visit', countryNodes[country], properties={
                        'purpose': 'pleasure'})
            redis_graph.add_edge(edge)

        redis_graph.commit()

        # delete nodes, to introduce deleted item within our datablock
        query = """MATCH (n:person) WHERE n.name = 'Roi' or n.name = 'Ailon' DELETE n"""
        redis_graph.query(query)

        query = """MATCH (n:country) WHERE n.name = 'USA' DELETE n"""
        redis_graph.query(query)

        # create indices
        actual_result = redis_con.execute_command(
            "GRAPH.QUERY", graph_name, "CREATE INDEX ON :person(name, height)")
        actual_result = redis_con.execute_command(
            "GRAPH.QUERY", graph_name, "CREATE INDEX ON :country(name, population)")

        return redis_graph

    def populate_dense_graph(self, graph_name):
        dense_graph = Graph(graph_name, redis_con)

        # return early if graph exists
        if redis_con.exists(graph_name):
            return dense_graph

        nodes = []
        for i in range(10):
            node = Node(label="n", properties={"val": i})
            dense_graph.add_node(node)
            nodes.append(node)

        for n_idx, n in enumerate(nodes):
            for m_idx, m in enumerate(nodes[:n_idx]):
                dense_graph.add_edge(Edge(n, "connected", m))

        dense_graph.flush()
        return dense_graph

    def test01_save_load_rdb(self):
        graph_names = ["G", "{tag}_G"]
        for graph_name in graph_names:
            graph = self.populate_graph(graph_name)
            for i in range(2):
                if i == 1:
                    # Save RDB & Load from RDB
                    self.env.dumpAndReload()

                # Verify
                # Expecting 5 person entities.
                query = """MATCH (p:person) RETURN COUNT(p)"""
                actual_result = graph.query(query)
                nodeCount = actual_result.result_set[0][0]
                self.env.assertEquals(nodeCount, 5)

                query = """MATCH (p:person) WHERE p.name='Alon' RETURN COUNT(p)"""
                actual_result = graph.query(query)
                nodeCount = actual_result.result_set[0][0]
                self.env.assertEquals(nodeCount, 1)

                # Expecting 3 country entities.
                query = """MATCH (c:country) RETURN COUNT(c)"""
                actual_result = graph.query(query)
                nodeCount = actual_result.result_set[0][0]
                self.env.assertEquals(nodeCount, 3)

                query = """MATCH (c:country) WHERE c.name = 'Israel' RETURN COUNT(c)"""
                actual_result = graph.query(query)
                nodeCount = actual_result.result_set[0][0]
                self.env.assertEquals(nodeCount, 1)

                # Expecting 2 visit edges.
                query = """MATCH (n:person)-[e:visit]->(c:country) WHERE e.purpose='pleasure' RETURN COUNT(e)"""
                actual_result = graph.query(query)
                edgeCount = actual_result.result_set[0][0]
                self.env.assertEquals(edgeCount, 2)

                # Verify indices exists
                expected_indices = [["exact-match", "country", ["name", "population"]], ["exact-match", "person", ["name", "height"]]]
                indices = graph.query("""CALL db.indexes()""").result_set
                self.env.assertEquals(indices, expected_indices)

    # Verify that edges are not modified after entity deletion
    def test02_deleted_entity_migration(self):
        graph_names = ("H", "{tag}_H")
        for graph_name in graph_names:
            graph = self.populate_dense_graph(graph_name)

            query = """MATCH (p) WHERE ID(p) = 0 OR ID(p) = 3 OR ID(p) = 7 OR ID(p) = 9 DELETE p"""
            actual_result = graph.query(query)
            self.env.assertEquals(actual_result.nodes_deleted, 4)

            query = """MATCH (p)-[]->(q) RETURN p.val, q.val ORDER BY p.val, q.val"""
            first_result = graph.query(query)

            # Save RDB & Load from RDB
            redis_con.execute_command("DEBUG", "RELOAD")

            second_result = graph.query(query)
            self.env.assertEquals(first_result.result_set,
                                second_result.result_set)

    # Strings, numerics, booleans, array, and point properties should be properly serialized and reloaded
    def test03_restore_properties(self):
        graph_names = ("simple_props", "{tag}_simple_props")
        for graph_name in graph_names:
            graph = Graph(graph_name, redis_con)

            query = """CREATE (:p {strval: 'str', numval: 5.5, boolval: true, array: [1,2,3], pointval: point({latitude: 5.5, longitude: 6})})"""
            result = graph.query(query)

            # Verify that node was created correctly
            self.env.assertEquals(result.nodes_created, 1)
            self.env.assertEquals(result.properties_set, 5)

            # Save RDB & Load from RDB
            redis_con.execute_command("DEBUG", "RELOAD")

            query = """MATCH (p) RETURN p.boolval, p.numval, p.strval, p.array, p.pointval"""
            actual_result = graph.query(query)

            # Verify that the properties are loaded correctly.
            expected_result = [[True, 5.5, 'str', [1, 2, 3], {"latitude": 5.5, "longitude": 6.0}]]
            self.env.assertEquals(actual_result.result_set, expected_result)

    # Verify multiple edges of the same relation between nodes A and B
    # are saved and restored correctly.
    def test04_repeated_edges(self):
        graph_names = ["repeated_edges", "{tag}_repeated_edges"]
        for graph_name in graph_names:
            graph = Graph(graph_name, redis_con)
            src   = Node(label='p', properties={'name': 'src'})
            dest  = Node(label='p', properties={'name': 'dest'})
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

    # Verify that graphs larger than the
    # default capacity are persisted correctly.
    def test05_load_large_graph(self):
        graph_name = "LARGE_GRAPH"
        graph = Graph(graph_name, redis_con)
        q = """UNWIND range(1, 50000) AS v CREATE (:L)-[:R {v: v}]->(:L)"""
        actual_result = graph.query(q)
        self.env.assertEquals(actual_result.nodes_created, 100_000)
        self.env.assertEquals(actual_result.relationships_created, 50_000)

        redis_con.execute_command("DEBUG", "RELOAD")
        
        expected_result = [[50000]]
        
        queries = [
            """MATCH (:L)-[r {v: 50000}]->(:L) RETURN r.v""",
            """MATCH (:L)-[r:R {v: 50000}]->(:L) RETURN r.v""",
            """MATCH ()-[r:R {v: 50000}]->() RETURN r.v"""
        ]

        for q in queries:
            actual_result = graph.query(q)
            self.env.assertEquals(actual_result.result_set, expected_result)
