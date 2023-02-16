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

        # scope of a subquery with no imports starts empty
        query = """
        MATCH (n:N)
        CALL {
            RETURN n.v AS INNERETURN
        }
        RETURN 1
        """
        self.expect_error(query, "n not defined")


        # make sure scope prior to the CALL {} is available after it
        res = graph.query(
            """
            UNWIND [0, 1, 2, 3] AS x
            CALL {
                WITH x
                MATCH (n {v: x})
                RETURN n
            }
            RETURN x
            """
        )
        self.env.assertEquals(res.result_set, [])

        # non-returned aliases should not be available
        query = """
        CALL {
            MATCH (n:N)
            RETURN 1 as innerReturn
        }
        RETURN n
        """
        self.expect_error(query, "n not defined")

    def test02_readonly_simple(self):
        """Test the simple read-only use-case of CALL {}, i.e., no updating
        clauses lay within the subquery"""

        # the graph is empty

        # create a node, and find it via CALL {}
        res = graph.query("CREATE (n:N {name: 'Raz'})")
        self.env.assertEquals(res.nodes_created, 1)

        # find a node via CALL {}
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

        # NTS: Clauses that can be used:
        # MATCH, OPTIONAL MATCH, WHERE, ORDERBY, SKIP, LIMIT, WITH, UNION,
        # UNWIND, (partly) FOREACH

        # add the `v` property to the one node currently existing, with val 4
        res = graph.query("MATCH (n) SET n.v = 4")
        self.env.assertEquals(res.properties_set, 1)

        # if a returning subquery doesn't return records, the input record is
        # not passed as well. Here, only one record should be returned
        res = graph.query(
            """
            CALL {
                UNWIND [1, 2, 3, 4] AS x
                MATCH (n:N {v: x})
                RETURN n
            }
            RETURN n
            """
        )

        # assert correctness of the result
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0],
            Node(label='N', properties={'name': 'Raz', 'v': 4}))

        # inner scope should be exposed to imported context ONLY via simple WITH
        query = """
                UNWIND [1, 2, 3, 4] AS x
                MATCH (n)
                CALL {
                    WITH n
                    RETURN x + 1
                }
                RETURN x + n.v
                """
        self.expect_error(query, "x not defined")

        # use returned value from the sq
        res = graph.query(
            """
            UNWIND [1, 2, 3, 4] AS x
            MATCH (n)
            CALL {
                WITH n
                RETURN n.v as INNERETURN
            }
            RETURN x + INNERETURN
            """
        )
        self.env.assertEquals(res.result_set, [[5], [6], [7], [8]])

        # # first input record to sq yields no records, next inputs do
        # res = graph.query(
        #     """
        #     UNWIND ['Omer', 'Raz', 'Moshe'] as name
        #     CALL {
        #         WITH name
        #         MATCH (n:N {name: name})
        #         RETURN n
        #     }
        #     RETURN n
        #     """
        # )
        # # assert correctness of the result
        # self.env.assertEquals(len(res.result_set), 1)
        # self.env.assertEquals(res.result_set[0][0],
        #     Node(label='N', properties={'name': 'Raz'}))


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
