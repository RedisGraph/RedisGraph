from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

redis_graph = None
dis_redis = None
redis_con = None
values = ["str1", "str2", False, True, 5, 10.5]

class testValueComparison(FlowTestsBase):
    def __init__(self):
        super(testValueComparison, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    @classmethod
    def populate_graph(self):
        global redis_graph

        for v in values:
            node = Node(label="value", properties={"val": v})
            redis_graph.add_node(node)

        # Add an additional node with no properties
        redis_graph.add_node(Node(label="value"))

        redis_graph.commit()

    # Verify the ordering of values that can and cannot be directly compared
    def test_orderability(self):
        query = """MATCH (v:value) RETURN v.val ORDER BY v.val"""
        actual_result = redis_graph.query(query)
        expected = [['str1'],
                    ['str2'],
                    [False],
                    [True],
                    [5],
                    [10.5],
                    [None]]
        self.env.assertEquals(actual_result.result_set, expected)

        # Expect the results to appear in reverse when using descending order
        query = """MATCH (v:value) RETURN v.val ORDER BY v.val DESC"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected[::-1])

    # From the Cypher specification:
    # "In a mixed set, any numeric value is always considered to be higher than any string value"
    def test_mixed_type_min(self):
        query = """MATCH (v:value) RETURN MIN(v.val)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], 'str1')

    def test_mixed_type_max(self):
        query = """MATCH (v:value) RETURN MAX(v.val)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], 10.5)

    # Verify that disjoint types pass <> filters
    def test_disjoint_comparisons(self):
        # Compare all node pairs under a Cartesian product
        query = """MATCH (v:value), (w:value) WHERE ID(v) <> ID(w) AND v.val = w.val RETURN v"""
        actual_result = redis_graph.query(query)
        # No nodes have the same property, so there should be 0 equal results
        expected_result_count = 0
        self.env.assertEquals(len(actual_result.result_set), expected_result_count)

        query = """MATCH (v:value), (w:value) WHERE ID(v) <> ID(w) AND v.val <> w.val RETURN v"""
        actual_result = redis_graph.query(query)
        # Every comparison should produce an inequal result
        node_count = len(redis_graph.nodes)
        expected_result_count = node_count * (node_count - 1)
        self.env.assertEquals(len(actual_result.result_set), expected_result_count)

    # Verify that AND conditions on true, false, and NULL values evaluate appropriately
    def test_AND_truth_tables(self):
        # Test two non-NULL values
        vals = [True, False]
        for a_val in vals:
            for b_val in vals:
                query = "RETURN %s AND %s" % (str(a_val), str(b_val))
                actual_result = redis_graph.query(query)
                expected_val = a_val and b_val
                self.env.assertEquals(actual_result.result_set[0][0], expected_val)

        # Test one NULL value
        for a_val in vals:
            query = "RETURN %s AND NULL" % (str(a_val))
            actual_result = redis_graph.query(query)
            # AND comparisons with one NULL value always evaluate to false
            expected_val = False
            self.env.assertEquals(actual_result.result_set[0][0], expected_val)

        # Test two NULL values
        query = "RETURN NULL AND NULL"
        actual_result = redis_graph.query(query)
        # AND comparisons with two NULL values evaluate to NULL
        self.env.assertEquals(actual_result.result_set[0][0], None)

    # Verify that OR conditions on true, false, and NULL values evaluate appropriately
    def test_OR_truth_tables(self):
        # Test two non-NULL values
        vals = [True, False]
        for a_val in vals:
            for b_val in vals:
                query = "RETURN %s OR %s" % (str(a_val), str(b_val))
                actual_result = redis_graph.query(query)
                expected_val = a_val or b_val
                self.env.assertEquals(actual_result.result_set[0][0], expected_val)

        # false OR null == null
        query = "RETURN false OR NULL"
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

        # true OR null == true
        query = "RETURN true OR NULL"
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], True)
        # null OR null == null
        query = "RETURN NULL OR NULL"
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)

    # Verify that XOR conditions on true, false, and NULL values evaluate appropriately
    def test_XOR_truth_tables(self):
        # Test two non-NULL values
        vals = [True, False]
        for a_val in vals:
            for b_val in vals:
                query = "RETURN %s XOR %s" % (str(a_val), str(b_val))
                actual_result = redis_graph.query(query)
                expected_val = a_val != b_val
                self.env.assertEquals(actual_result.result_set[0][0], expected_val)

        # one NULL value
        for a_val in vals:
            query = "RETURN %s XOR NULL" % (str(a_val))
            actual_result = redis_graph.query(query)
            # XOR comparisons with one NULL value always evaluate to null
            expected_val = None
            self.env.assertEquals(actual_result.result_set[0][0], expected_val)

        # Test two NULL values
        query = "RETURN NULL XOR NULL"
        actual_result = redis_graph.query(query)
        # XOR comparisons with two NULL values evaluate to NULL
        self.env.assertEquals(actual_result.result_set[0][0], None)

    # Verify that NOT conditions on true, false, and NULL values evaluate appropriately
    def test_NOT_truth_tables(self):
        # Test non-NULL values
        vals = [True, False]
        for a_val in vals:
            query = "RETURN NOT %s" % (str(a_val))
            actual_result = redis_graph.query(query)
            expected_val = not a_val
            self.env.assertEquals(actual_result.result_set[0][0], expected_val)

        # NOT null == null
        query = "RETURN NOT NULL"
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set[0][0], None)
