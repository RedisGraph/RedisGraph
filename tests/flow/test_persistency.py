from common import *
import random
from index_utils import *
from click.testing import CliRunner
from redisgraph_bulk_loader.bulk_insert import bulk_insert

redis_con = None
port = None

class testGraphPersistency():
    def __init__(self):
        self.env = Env(decodeResponses=True, enableDebugCommand=True)

        # skip test if we're running under Sanitizer
        if SANITIZER != "":
            self.env.skip() # sanitizer is not working correctly with bulk

        global redis_con
        redis_con = self.env.getConnection()
        global port
        port = self.env.envRunner.port
        

    def populate_graph(self, graph_name):
        redis_graph = Graph(redis_con, graph_name)
        # quick return if graph already exists
        if redis_con.exists(graph_name):
            return redis_graph

        people       = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]
        visits       = [("Roi", "USA"), ("Alon", "Israel"), ("Ailon", "Japan"), ("Boaz", "United Kingdom")]
        countries    = ["Israel", "USA", "Japan", "United Kingdom"]
        personNodes  = {}
        countryNodes = {}

        # create nodes
        for p in people:
            person = Node(label="person", properties={"name": p, "height": random.randint(160, 200)})
            redis_graph.add_node(person)
            personNodes[p] = person

        for c in countries:
            country = Node(label="country", properties={"name": c, "population": random.randint(100, 400)})
            redis_graph.add_node(country)
            countryNodes[c] = country

        # create edges
        for v in visits:
            person  = v[0]
            country = v[1]
            edge = Edge(personNodes[person], 'visit', countryNodes[country], properties={
                        'purpose': 'pleasure'})
            redis_graph.add_edge(edge)

        redis_graph.commit()

        # delete nodes, to introduce deleted entries within our datablock
        query = """MATCH (n:person) WHERE n.name = 'Roi' or n.name = 'Ailon' DELETE n"""
        redis_graph.query(query)

        query = """MATCH (n:country) WHERE n.name = 'USA' DELETE n"""
        redis_graph.query(query)

        # create indices
        create_node_exact_match_index(redis_graph, "person", "name", "height")
        create_node_exact_match_index(redis_graph, "country", "name", "population")
        create_edge_exact_match_index(redis_graph, "visit", "purpose")
        redis_graph.query("CALL db.idx.fulltext.createNodeIndex({label: 'person', stopwords: ['A', 'B'], language: 'english'}, { field: 'text', nostem: true, weight: 2, phonetic: 'dm:en' })")
        wait_for_indices_to_sync(redis_graph)

        return redis_graph

    def populate_dense_graph(self, graph_name):
        dense_graph = Graph(redis_con, graph_name)

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

    def test01_save_load(self):
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
                indices = graph.query("""CALL db.indexes()""").result_set
                expected_indices = [
                        ['exact-match', 'country', ['name', 'population'], 'english', [], 'NODE'],
                        ['exact-match', 'person', ['name', 'height'], 'english', [], 'NODE'],
                        ['full-text', 'person', ['text'], 'english', ['a', 'b'], 'NODE'],
                        ['exact-match', 'visit', ['_src_id', '_dest_id', 'purpose'], 'english', [], 'RELATIONSHIP']
                ]

                self.env.assertEquals(len(indices), len(expected_indices))
                for index in indices:
                    self.env.assertIn(index, indices)

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
            self.env.dumpAndReload()

            second_result = graph.query(query)
            self.env.assertEquals(first_result.result_set,
                                  second_result.result_set)

    # Strings, numerics, booleans, array, and point properties should be properly serialized and reloaded
    def test03_restore_properties(self):
        graph_names = ("simple_props", "{tag}_simple_props")
        for graph_name in graph_names:
            graph = Graph(redis_con, graph_name)

            query = """CREATE (:p {strval: 'str', numval: 5.5, boolval: true, array: [1,2,3], pointval: point({latitude: 5.5, longitude: 6})})"""
            result = graph.query(query)

            # Verify that node was created correctly
            self.env.assertEquals(result.nodes_created, 1)
            self.env.assertEquals(result.properties_set, 5)

            # Save RDB & Load from RDB
            self.env.dumpAndReload()

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
            graph = Graph(redis_con, graph_name)
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
            self.env.dumpAndReload()

            # Verify that the latest edge was properly saved and loaded
            actual_result = graph.query(q)
            self.env.assertEquals(actual_result.result_set, expected_result)

    # Verify that graphs larger than the
    # default capacity are persisted correctly.
    def test05_load_large_graph(self):
        graph_name = "LARGE_GRAPH"
        graph = Graph(redis_con, graph_name)
        q = """UNWIND range(1, 50000) AS v CREATE (:L)-[:R {v: v}]->(:L)"""
        actual_result = graph.query(q)
        self.env.assertEquals(actual_result.nodes_created, 100_000)
        self.env.assertEquals(actual_result.relationships_created, 50_000)

        # Save RDB & Load from RDB
        self.env.dumpAndReload()

        expected_result = [[50000]]

        queries = [
            """MATCH (:L)-[r {v: 50000}]->(:L) RETURN r.v""",
            """MATCH (:L)-[r:R {v: 50000}]->(:L) RETURN r.v""",
            """MATCH ()-[r:R {v: 50000}]->() RETURN r.v"""
        ]

        for q in queries:
            actual_result = graph.query(q)
            self.env.assertEquals(actual_result.result_set, expected_result)

    # Verify that graphs created using the GRAPH.BULK endpoint are persisted correctly
    def test06_bulk_insert(self):
        graphname = "bulk_inserted_graph"
        runner = CliRunner()

        csv_path = os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/resources/bulk_formatted/'
        res = runner.invoke(bulk_insert, ['--port', port,
                                          '--nodes', csv_path + 'Person.csv',
                                          '--nodes', csv_path + 'Country.csv',
                                          '--relations', csv_path + 'KNOWS.csv',
                                          '--relations', csv_path + 'VISITED.csv',
                                          graphname])

        # The script should report 27 node creations and 56 edge creations
        self.env.assertEquals(res.exit_code, 0)
        self.env.assertIn('27 nodes created', res.output)
        self.env.assertIn('56 relations created', res.output)

        # Restart the server
        self.env.dumpAndReload()

        graph = Graph(redis_con, graphname)

        query_result = graph.query("""MATCH (p:Person)
                                      RETURN p.name, p.age, p.gender, p.status, ID(p)
                                      ORDER BY p.name""")

        # Verify that the Person label exists, has the correct attributes
        # and is properly populated
        expected_result = [
                ['Ailon Velger',         32,     'male',    'married',  2],
                ['Alon Fital',           32,     'male',    'married',  1],
                ['Boaz Arad',            31,     'male',    'married',  4],
                ['Gal Derriere',         26,     'male',    'single',   11],
                ['Jane Chernomorin',     31,     'female',  'married',  8],
                ['Lucy Yanfital',        30,     'female',  'married',  7],
                ['Mor Yesharim',         31,     'female',  'married',  12],
                ['Noam Nativ',           34,     'male',    'single',   13],
                ['Omri Traub',           33,     'male',    'single',   5],
                ['Ori Laslo',            32,     'male',    'married',  3],
                ['Roi Lipman',           32,     'male',    'married',  0],
                ['Shelly Laslo Rooz',    31,     'female',  'married',  9],
                ['Tal Doron',            32,     'male',    'single',   6],
                ['Valerie Abigail Arad', 31,     'female',  'married',  10]
                ]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Verify that the Country label exists, has the correct attributes, and is properly populated
        query_result = graph.query('MATCH (c:Country) RETURN c.name, ID(c) ORDER BY c.name')
        expected_result = [
                ['Andora',       21],
                ['Canada',       18],
                ['China',        19],
                ['Germany',      24],
                ['Greece',       17],
                ['Italy',        25],
                ['Japan',        16],
                ['Kazakhstan',   22],
                ['Netherlands',  20],
                ['Prague',       15],
                ['Russia',       23],
                ['Thailand',     26],
                ['USA',          14]
        ]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Validate that the expected relations and properties have been constructed
        query_result = graph.query('MATCH (a)-[e:KNOWS]->(b) RETURN a.name, e.relation, b.name ORDER BY e.relation, a.name, b.name')

        expected_result = [
                ['Ailon Velger', 'friend',   'Noam Nativ'],
                ['Alon Fital',   'friend',   'Gal Derriere'],
                ['Alon Fital',   'friend',   'Mor Yesharim'],
                ['Boaz Arad',    'friend',   'Valerie Abigail Arad'],
                ['Roi Lipman',   'friend',   'Ailon Velger'],
                ['Roi Lipman',   'friend',   'Alon Fital'],
                ['Roi Lipman',   'friend',   'Boaz Arad'],
                ['Roi Lipman',   'friend',   'Omri Traub'],
                ['Roi Lipman',   'friend',   'Ori Laslo'],
                ['Roi Lipman',   'friend',   'Tal Doron'],
                ['Ailon Velger', 'married',  'Jane Chernomorin'],
                ['Alon Fital',   'married',  'Lucy Yanfital'],
                ['Ori Laslo',    'married',  'Shelly Laslo Rooz']
        ]
        self.env.assertEquals(query_result.result_set, expected_result)

        query_result = graph.query('MATCH (a)-[e:VISITED]->(b) RETURN a.name, e.purpose, b.name ORDER BY e.purpose, a.name, b.name')

        expected_result = [
                ['Alon Fital',           'business',  'Prague'],
                ['Alon Fital',           'business',  'USA'],
                ['Boaz Arad',            'business',  'Netherlands'],
                ['Boaz Arad',            'business',  'USA'],
                ['Gal Derriere',         'business',  'Netherlands'],
                ['Jane Chernomorin',     'business',  'USA'],
                ['Lucy Yanfital',        'business',  'USA'],
                ['Mor Yesharim',         'business',  'Germany'],
                ['Ori Laslo',            'business',  'China'],
                ['Ori Laslo',            'business',  'USA'],
                ['Roi Lipman',           'business',  'Prague'],
                ['Roi Lipman',           'business',  'USA'],
                ['Tal Doron',            'business',  'Japan'],
                ['Tal Doron',            'business',  'USA'],
                ['Alon Fital',           'pleasure',  'Greece'],
                ['Alon Fital',           'pleasure',  'Prague'],
                ['Alon Fital',           'pleasure',  'USA'],
                ['Boaz Arad',            'pleasure',  'Netherlands'],
                ['Boaz Arad',            'pleasure',  'USA'],
                ['Jane Chernomorin',     'pleasure',  'Greece'],
                ['Jane Chernomorin',     'pleasure',  'Netherlands'],
                ['Jane Chernomorin',     'pleasure',  'USA'],
                ['Lucy Yanfital',        'pleasure',  'Kazakhstan'],
                ['Lucy Yanfital',        'pleasure',  'Prague'],
                ['Lucy Yanfital',        'pleasure',  'USA'],
                ['Mor Yesharim',         'pleasure',  'Greece'],
                ['Mor Yesharim',         'pleasure',  'Italy'],
                ['Noam Nativ',           'pleasure',  'Germany'],
                ['Noam Nativ',           'pleasure',  'Netherlands'],
                ['Noam Nativ',           'pleasure',  'Thailand'],
                ['Omri Traub',           'pleasure',  'Andora'],
                ['Omri Traub',           'pleasure',  'Greece'],
                ['Omri Traub',           'pleasure',  'USA'],
                ['Ori Laslo',            'pleasure',  'Canada'],
                ['Roi Lipman',           'pleasure',  'Japan'],
                ['Roi Lipman',           'pleasure',  'Prague'],
                ['Shelly Laslo Rooz',    'pleasure',  'Canada'],
                ['Shelly Laslo Rooz',    'pleasure',  'China'],
                ['Shelly Laslo Rooz',    'pleasure',  'USA'],
                ['Tal Doron',            'pleasure',  'Andora'],
                ['Tal Doron',            'pleasure',  'USA'],
                ['Valerie Abigail Arad', 'pleasure',  'Netherlands'],
                ['Valerie Abigail Arad', 'pleasure',  'Russia']
                ]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Verify that nodes with multiple labels are saved and restored correctly.
    def test07_persist_multiple_labels(self):
        graph_id = "multiple_labels"
        g = Graph(redis_con, graph_id)
        q = "CREATE (a:L0:L1:L2)"
        actual_result = g.query(q)
        self.env.assertEquals(actual_result.nodes_created, 1)
        self.env.assertEquals(actual_result.labels_added, 3)

        # Verify the new node
        q = "MATCH (a) RETURN LABELS(a)"
        actual_result = g.query(q)
        expected_result = [[["L0", "L1", "L2"]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Save RDB & Load from RDB
        self.env.dumpAndReload()

        # Verify that the graph was properly saved and loaded
        actual_result = g.query(q)
        self.env.assertEquals(actual_result.result_set, expected_result)

        queries = [
                "MATCH (a:L0) RETURN count(a)",
                "MATCH (a:L1) RETURN count(a)",
                "MATCH (a:L2) RETURN count(a)",
                "MATCH (a:L0:L0) RETURN count(a)",
                "MATCH (a:L0:L1) RETURN count(a)",
                "MATCH (a:L0:L2) RETURN count(a)",
                "MATCH (a:L1:L0) RETURN count(a)",
                "MATCH (a:L1:L1) RETURN count(a)",
                "MATCH (a:L1:L2) RETURN count(a)",
                "MATCH (a:L2:L0) RETURN count(a)",
                "MATCH (a:L2:L1) RETURN count(a)",
                "MATCH (a:L2:L2) RETURN count(a)",
                "MATCH (a:L0:L1:L2) RETURN count(a)"]

        for q in queries:
            actual_result = g.query(q)
            self.env.assertEquals(actual_result.result_set[0], [1])
