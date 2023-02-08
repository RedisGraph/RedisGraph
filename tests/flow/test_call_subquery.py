from common import *
from collections import Counter

graph = None
GRAPH_ID = "G"

class testCallSubqueryFlow():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, GRAPH_ID)
    
    def get_res_and_assertEquals(self, query, expected_result):
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)
        return actual_result

    def expect_error(self, query, expected_err_msg):
        try:
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn(expected_err_msg, str(e))

    # validate the non-valid queries don't pass validations
    def test01_test_validations(self):
        # non-simple imports
        for query in [
            "WITH 1 AS a CALL {WITH 1 AS a RETURN 1} RETURN 1",
            "WITH 1 AS a CALL {WITH a+1 AS a RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a AS b RETURN b} RETURN b",
            "WITH 1 AS a CALL {WITH a LIMIT 5 RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a ORDER BY a.v RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a WHERE a > 5 RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a SKIP 5 RETURN a} RETURN a",
        ]:
            self.expect_error(query,
                "WITH imports in CALL {} must be simple ('WITH a')")

        # non-valid queries within CAll {}
        for query in [
            "CALL {CREATE (n:N) MATCH (n:N) RETURN n} RETURN 1",
            "WITH 1 AS a CALL {WITH a CREATE (n:N) MATCH (n:N) RETURN n} RETURN a",
            "CALL {MATCH (n:N) CREATE (n:N2)} RETURN 1"
        ]:
            # just pass in case of an error, fail otherwise
            self.expect_error(query, "")

    def test02_readonly_simple(self):
        """Test the simple read-only use-case of CALL {}, i.e., no updating
        clauses lay within the subquery"""

        # the graph is empty

        # create a node, and find it via CALL {}
        res = graph.query("CREATE (n:N {name: 'Raz'})")
        self.env.assertEquals(res.nodes_created, 1)

        # find the node via CALL {}
        res = graph.query(
            """CALL
                {
                    MATCH (n:N {name: 'Raz'}) RETURN n
                }
                RETURN n
            """
            )
        # one node should have been found
        self.env.assertEquals(len(res.result_set), 1)
        # node labels and props should match created node
        self.env.assertEquals(res.result_set[0][0],
                              Node(label='N', properties={'name': 'Raz'}))

    def test03_readonly(self):
        """RO returning/unit subqueries"""

        # the graph has one node with label `N` and prop `name` with val `Raz`

        # clauses that can be used:
        # MATCH, OPTIONAL MATCH, WHERE, ORDERBY, SKIP, LIMIT, WITH, UNION,
        # UNWIND, (partly) FOREACH

        res = graph.query(
            """
            UNWIND ['Omer', 'Raz', 'Moshe'] as name
            CALL {
                WITH name
                MATCH (n:N {name: name})
                RETURN n
            }
            RETURN n
            """
        )
        # assert correctness of the result
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set,
            Node(label='N', properties={'name': 'Raz'}))


    # def test04_update_no_return_simple(self):
    #     """simple updates"""

    #     # clean db
    #     self.env.flush()
    #     graph = Graph(self.env.getConnection(), GRAPH_ID)

    #     # add a node via CALL {}


    #     # update an edge via CALL {}


    #     # remove a node\edge label via CALL {}


    #     # remove a node\edge property via CALL {}


    #     # delete a node\edge via CALL {}
