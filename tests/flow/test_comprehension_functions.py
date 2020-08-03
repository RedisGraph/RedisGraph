from RLTest import Env
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None

class testComprehensionFunctions(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        graph_id = "list_comprehension"
        redis_con = self.env.getConnection()
        redis_graph = Graph(graph_id, redis_con)
        self.populate_graph()

    def populate_graph(self):
        global redis_graph

        # Construct a graph with the form:
        # (v1)-[e1]->(v2)-[e2]->(v3)
        node_props = ['v1', 'v2', 'v3']

        nodes = []
        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"val": v})
            nodes.append(node)
            redis_graph.add_node(node)

        edge = Edge(nodes[0], "E", nodes[1], properties={"edge_val": ['v1', 'v2']})
        redis_graph.add_edge(edge)

        edge = Edge(nodes[1], "E", nodes[2], properties={"edge_val": ['v2', 'v3']})
        redis_graph.add_edge(edge)

        redis_graph.commit()

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
        expected_result = [[[1, 2, 3]]]
        query = """WITH [1,2,3] AS arr RETURN [elem IN arr]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)
        query = """RETURN [elem IN [1,2,3]]"""
        actual_result = redis_graph.query(query)
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

        query = """MATCH p=()-[*]->() WITH nodes(p) AS nodes WITH [elem IN nodes | elem.val] AS vals RETURN vals ORDER BY vals"""
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
        query = """MATCH (n) WHERE n.val IN [x in ['v1', 'v3']] RETURN n.val ORDER BY n.val"""
        actual_result = redis_graph.query(query)
        expected_result = [['v1'],
                           ['v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test08_list_comprehension_on_property_array(self):
        query = """MATCH (n)-[e]->() WITH n, e ORDER BY n.val RETURN [elem IN e.edge_val WHERE elem = n.val]"""
        actual_result = redis_graph.query(query)
        expected_result = [[['v1']],
                           [['v2']]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test09_nested_list_comprehension(self):
        query = """RETURN [elem IN [nested_val IN range(0, 6) WHERE nested_val % 2 = 0] WHERE elem * 2 >= 4 | elem * 2]"""
        actual_result = redis_graph.query(query)
        expected_result = [[[4, 8, 12]]]
        self.env.assertEquals(actual_result.result_set, expected_result)
