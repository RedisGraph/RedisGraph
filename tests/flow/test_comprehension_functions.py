from RLTest import Env
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None

class testComprehensionFunctions(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    def populate_graph(self):
        global redis_graph

        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)
        node_props = ['v1', 'v2', 'v3']

        redis_graph.query("CREATE (:L {val: 'v1'})-[:E {edgeval: 1}]->(:L {val: 'v2'})-[:E {edgeval: 2}]->(:L {val: 'v3'})")
        # TODO functional, but creation is non-deterministic
        #  nodes = []
        #  for idx, v in enumerate(node_props):
            #  node = Node(label="L", properties={"val": v})
            #  nodes.append(node)
            #  redis_graph.add_node(node)

        #  edge = Edge(nodes[0], "E", nodes[1], properties={"edge_val": 1})
        #  redis_graph.add_edge(edge)

        #  edge = Edge(nodes[1], "E", nodes[2], properties={"edge_val": 2})
        #  redis_graph.add_edge(edge)

        #  redis_graph.commit()

    # Test list comprehension queries with scalar inputs and a single result row
    def test01_list_comprehension_single_return(self):
        expected_result = [[[2, 6]]]

        # Test logically identical queries that generate the same input array with different methods.
        query = """WITH [1,2,3] AS arr RETURN [elem IN arr WHERE elem % 2 = 1 | elem * 2]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """RETURN [elem IN [1,2,3] WHERE elem % 2 = 1 | elem * 2]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """RETURN [elem IN range(1,3) WHERE elem % 2 = 1 | elem * 2]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test02_list_comprehension_no_filter_no_map(self):
        query = """WITH [1,2,3] AS arr RETURN [elem IN arr]"""
        actual_result = redis_graph.query(query)
        expected_result = [[[1, 2, 3]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test03_list_comprehension_map_no_filter(self):
        query = """WITH [1,2,3] AS arr RETURN [elem IN arr | elem * 2]"""
        actual_result = redis_graph.query(query)
        expected_result = [[[2, 4, 6]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test04_list_comprehension_filter_no_map(self):
        query = """WITH [1,2,3] AS arr RETURN [elem IN arr WHERE elem % 2 = 1]"""
        actual_result = redis_graph.query(query)
        expected_result = [[[1, 3]]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test05_list_comprehension_on_allocated_values(self):
        query = """WITH [toUpper('str1'), toUpper('str2'), toUpper('str3')] AS arr RETURN [elem IN arr]"""
        actual_result = redis_graph.query(query)
        expected_result = [[['STR1', 'STR2', 'STR3']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """WITH [toUpper('str1'), toUpper('str2'), toUpper('str3')] AS arr RETURN [elem IN arr WHERE toLower(elem) = 'str2']"""
        actual_result = redis_graph.query(query)
        expected_result = [[['STR2']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """WITH [toUpper('str1'), toUpper('str2'), toUpper('str3')] AS arr RETURN [elem IN arr WHERE toLower(elem) = 'str2' | elem + 'low']"""
        actual_result = redis_graph.query(query)
        expected_result = [[['STR2low']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test06_list_comprehension_on_graph_entities(self):
        query = """MATCH p=()-[*]->() WITH nodes(p) AS nodes RETURN [elem IN nodes]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), 3)

        query = """MATCH p=()-[*]->() WITH nodes(p) AS nodes RETURN [elem IN nodes | elem.val]"""
        actual_result = redis_graph.query(query)
        expected_result = [[['v1', 'v2']],
                           [['v1', 'v2', 'v3']],
                           [['v2', 'v3']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[*]->() WITH nodes(p) AS nodes RETURN [elem IN nodes WHERE elem.val = 'v2' | elem.val]"""
        actual_result = redis_graph.query(query)
        expected_result = [[['v2']],
                           [['v2']],
                           [['v2']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH p=()-[*]->() WITH nodes(p) AS nodes RETURN [elem IN nodes WHERE elem.val = 'v2' | elem.val + 'a']"""
        actual_result = redis_graph.query(query)
        expected_result = [[['v2a']],
                           [['v2a']],
                           [['v2a']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test07_list_comprehension_in_where_predicate(self):
        query = """MATCH (n) WHERE n.val IN [x in ['v1', 'v3']] RETURN n.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1'],
                           ['v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)
