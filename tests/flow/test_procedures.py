import os
import sys
import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "procedures"
redis_graph = None
redis_con = None

node1 = Node(label="fruit", properties={"name": "Orange1", "value": 1})
node2 = Node(label="fruit", properties={"name": "Orange2", "value": 2})
node3 = Node(label="fruit", properties={"name": "Orange3", "value": 3})
node4 = Node(label="fruit", properties={"name": "Orange4", "value": 4})
node5 = Node(label="fruit", properties={"name": "Banana", "value": 5})

# Tests built in procedures,
# e.g. db.idx.fulltext.queryNodes
# Test over all procedure behavior in addition to procedure specifics.
class testProcedures(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        if redis_con.exists(GRAPH_ID):
            return

        edge = Edge(node1, 'goWellWith', node5)
        redis_graph.add_node(node1)
        redis_graph.add_node(node2)
        redis_graph.add_node(node3)
        redis_graph.add_node(node4)
        redis_graph.add_node(node5)
        redis_graph.add_edge(edge)
        redis_graph.commit()

        # Create full-text index.
        redis_graph.call_procedure("db.idx.fulltext.createNodeIndex", 'fruit', 'name')

    # Compares two nodes based on their properties.
    def _compareNodes(self, a, b):
        return a.properties == b.properties

    # Make sure given item is found within resultset.
    def _inResultSet(self, item, resultset):
        for i in range(len(resultset)):
            result = resultset[i][0]
            if self._compareNodes(item, result):
                return True
        return False

    # Issue query and validates resultset.
    def queryAndValidate(self, query, expected_results, query_params={}):
        actual_resultset = redis_graph.query(query, query_params).result_set
        self.env.assertEquals(len(actual_resultset), len(expected_results))
        for i in range(len(actual_resultset)):
            self.env.assertTrue(self._inResultSet(expected_results[i], actual_resultset))
    
    # Call procedure, omit yield, expecting all procedure outputs to
    # be included in result-set.
    def test_no_yield(self):
        actual_result = redis_graph.call_procedure("db.idx.fulltext.queryNodes", "fruit", "Orange1")
        assert(len(actual_result.result_set) is 1)

        header = actual_result.header
        data = actual_result.result_set[0]
        assert(header[0][1] == 'node')
        assert(data[0] is not None)

    # Call procedure specify different outputs.
    def test_yield(self):
        actual_result = redis_graph.call_procedure("db.idx.fulltext.queryNodes", "fruit", "Orange1", y=["node"])
        assert(len(actual_result.result_set) is 1)

        header = actual_result.header
        data = actual_result.result_set[0]
        assert(header[0][1] == 'node')
        assert(data[0] is not None)

        # Yield an unknown output.
        # Expect an error when trying to use an unknown procedure output.
        try:
            redis_graph.call_procedure("db.idx.fulltext.queryNodes", "fruit", "Orange1", y=["unknown"])
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass
        
        # Yield the same output multiple times.
        # Expect an error when trying to use the same output multiple times.
        try:
            redis_graph.call_procedure("db.idx.fulltext.queryNodes", "fruit", "Orange1", y=["node", "node"])
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass
    
    def test_arguments(self):
        # Omit arguments.
        # Expect an error when trying to omit arguments.
        try:
            redis_graph.call_procedure("db.idx.fulltext.queryNodes")
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass
        
        # Omit arguments, queryNodes expecting 2 argument, provide 1.
        # Expect an error when trying to omit arguments.
        try:
            redis_graph.call_procedure("db.idx.fulltext.queryNodes", "arg1")
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

        # Overload arguments.
        # Expect an error when trying to send too many arguments.
        try:
            redis_graph.call_procedure("db.idx.fulltext.queryNodes", "fruit", "query", "fruit", "query", y=["node"])
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Test procedure call while mixing a number of addition clauses.
    def test_mix_clauses(self):
        query_params = {'prefix': 'Orange*'}
        # CALL + RETURN.

        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    RETURN node"""
        expected_results = [node4, node2, node3, node1]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # The combination of CALL and WHERE currently creates a syntax error in libcypher-parser.
        # CALL + WHERE + RETURN + ORDER.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    """
        expected_results = [node3, node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + WHERE + RETURN + ORDER + SKIP.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    SKIP 1"""
        expected_results = [node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + WHERE + RETURN + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    LIMIT 2"""
        expected_results = [node3, node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + WHERE + RETURN + ORDER + SKIP + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    SKIP 1
                    LIMIT 1"""
        expected_results = [node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)

        # CALL + RETURN + ORDER.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    RETURN node
                    ORDER BY node.value
                    """
        expected_results = [node1, node2, node3, node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + RETURN + ORDER + SKIP.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    RETURN node
                    ORDER BY node.value
                    SKIP 1
                    """
        expected_results = [node2, node3, node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + RETURN + ORDER + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    RETURN node
                    ORDER BY node.value
                    LIMIT 2
                    """
        expected_results = [node1, node2]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + RETURN + ORDER + SKIP + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    RETURN node
                    ORDER BY node.value
                    SKIP 1
                    LIMIT 1
                    """
        expected_results = [node2]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + WHERE + RETURN + ORDER.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value"""
        expected_results = [node3, node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + WHERE + RETURN + ORDER + SKIP.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    SKIP 1"""
        expected_results = [node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + WHERE + RETURN + ORDER + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    LIMIT 1"""
        expected_results = [node3]
        self.queryAndValidate(query, expected_results, query_params=query_params)


        # CALL + WHERE + RETURN + ORDER + SKIP + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    SKIP 1
                    LIMIT 1"""
        expected_results = [node4]
        self.queryAndValidate(query, expected_results, query_params=query_params)

        # CALL + MATCH + RETURN.
        query = """CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node
            MATCH (node)-[]->(z)
            RETURN z"""
        expected_results = [node5]
        self.queryAndValidate(query, expected_results, query_params=query_params)

        # UNWIND + CALL + RETURN.
        query = """UNWIND([1,2]) AS x CALL db.idx.fulltext.queryNodes('fruit', $prefix) YIELD node RETURN node"""
        expected_results = [node4, node2, node3, node1, node4, node2, node3, node1]
        self.queryAndValidate(query, expected_results, query_params=query_params)

    def test_procedure_labels(self):
        actual_resultset = redis_graph.call_procedure("db.labels").result_set
        expected_results = [["fruit"]]        
        self.env.assertEquals(actual_resultset, expected_results)
    
    def test_procedure_relationshipTypes(self):
        actual_resultset = redis_graph.call_procedure("db.relationshipTypes").result_set
        expected_results = [["goWellWith"]]
        self.env.assertEquals(actual_resultset, expected_results)
    
    def test_procedure_propertyKeys(self):
        actual_resultset = redis_graph.call_procedure("db.propertyKeys").result_set
        expected_results = [["name"], ["value"]]
        self.env.assertEquals(actual_resultset, expected_results)

    def test_procedure_fulltext_syntax_error(self):
        try:
            query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange || Apple') YIELD node RETURN node"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass
