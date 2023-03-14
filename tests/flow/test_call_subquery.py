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

    def test02_simple_scan_return(self):
        """Test a simple scan and return subquery"""

        # the graph is empty
        # create a node, and find it via CALL {}
        res = graph.query("CREATE (n:N {name: 'Raz'})")
        self.env.assertEquals(res.nodes_created, 1)

        # find a node via CALL {}
        res = graph.query(
            """CALL
                {
                    MATCH (n:N {name: 'Raz'})
                    RETURN n
                }
                RETURN n
            """
            )
        # one node should have been found
        self.env.assertEquals(len(res.result_set), 1)
        # node labels and props should match created node
        self.env.assertEquals(res.result_set[0][0],
                              Node(label='N', properties={'name': 'Raz'}))

    def test03_filter_results(self):
        """Test that no records return from the subquery for scans that found
        nothing."""

        # the graph has one node with label `N` and prop `name` with val `Raz`
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

    def test04_reference_innerReturn_alias(self):
        """Test that the outer scope can reference the returned projections
        from the subquery"""

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

    def test05_many_to_one_not_first(self):
        """Test the case Call {} gets several records, and returns only one,
        that corresponds to a record that is not the first one it gets"""

                # first input record to sq yields no records, next inputs do
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
        self.env.assertEquals(res.result_set[0][0],
            Node(label='N', properties={'name': 'Raz', 'v': 4}))

    def test06_many_to_one(self):
        """Test the case Call {} changes the amount of records such that there
        are more output records than input"""

        # the graph has one node with label `N` and prop `name` with val `Raz`
        # with the `v` property with value 4. Create another node with `v` set
        # to 1
        graph.query("CREATE (:N {name: 'Raz', v: 1})")

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

        # validate the results
        self.env.assertEquals(len(res.result_set), 2)
        self.env.assertEquals(res.result_set[0][0],
        Node(label='N', properties={'name': 'Raz', 'v': 1}))
        self.env.assertEquals(res.result_set[1][0],
        Node(label='N', properties={'name': 'Raz', 'v': 4}))

    def test07_optional_match(self):
        """test that OPTIONAL MATCH within the subquery works appropriately,
        i.e., passes records with empty columns in case no match was found"""

        res = graph.query(
            """
            CALL {
                UNWIND [1, 2, 3, 4] AS x
                OPTIONAL MATCH (n:N {v: x})
                RETURN n
            }
            RETURN n
            """
        )

        # validate the results
        self.env.assertEquals(len(res.result_set), 4)
        self.env.assertEquals(res.result_set[0][0],
        Node(label='N', properties={'name': 'Raz', 'v': 1}))
        self.env.assertEquals(res.result_set[3][0],
        Node(label='N', properties={'name': 'Raz', 'v': 4}))

    def test08_filtering(self):
        """Test that filtering within the subquery works fine"""

        # the graph has two nodes, with `name` 'Raz' and `v` 1 and 4
        res = graph.query(
            """
            CALL {
                MATCH (n:N)
                WHERE n.v = 1
                RETURN n
            }
            RETURN n
            """
        )

        # assert the correctness of the results
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0],
        Node(label='N', properties={'name': 'Raz', 'v': 1}))

    def test09_simple_ret_eager(self):
        """Simple test for the returning eager subquery case"""

        # the graph has two nodes, with `name` 'Raz' and `v` 1 and 4
        # execute a query with an eager and returning subquery inside
        res = graph.query(
            """
            CALL {
                MATCH (n:N)
                SET n.v = n.v + 1
                RETURN n
            }
            RETURN n ORDER BY n.v ASC
            """
        )

        # make sure the wanted actions occurred
        self.env.assertEquals(res.properties_set, 2)
        self.env.assertEquals(res.result_set[0][0], Node(label='N', properties={'name': 'Raz', 'v': 2}))
        self.env.assertEquals(res.result_set[1][0], Node(label='N', properties={'name': 'Raz', 'v': 5}))

        # import outer-scope aliases, reference them inside and outside the sq
        res = graph.query(
            """
            WITH 2 AS x
            CALL {
                WITH x
                MATCH (n:N {v: x})
                SET n.v = 2 * n.v
                RETURN n
            }
            RETURN n, x
            """
        )

        # make sure the wanted actions occurred
        self.env.assertEquals(res.properties_set, 1)
        self.env.assertEquals(res.result_set[0][0], Node(label='N', properties={'name': 'Raz', 'v': 4}))
        self.env.assertEquals(res.result_set[0][1], 2)

    # test FOREACH within CALL {} (updating (eager), returning)
    def test10_foreach(self):
        """Test that FOREACH works properly when used inside a subquery"""

        # the graph has two nodes, with `name` 'Raz' and `v` 4 and 5

        res = graph.query(
            """
            CALL {
                MATCH (n:N)
                FOREACH (i in [1] |
                    SET n.v = n.v + 1
                )
                RETURN n
            }
            RETURN n ORDER BY n.v ASC
            """
        )

        # assert the correctness of the results
        self.env.assertEquals(res.result_set[0][0],
            Node(label='N', properties={'name': 'Raz', 'v': 5}))
        self.env.assertEquals(res.result_set[1][0],
            Node(label='N', properties={'name': 'Raz', 'v': 6}))

        # TODO: add another query with a non-returning subquery

    # tests that SKIP and LIMIT work properly
    def test11_skip_limit(self):
        """Test that SKIP works properly when placed inside a subquery"""

        # the graph has two nodes, with `name` 'Raz' and `v` 5 and 6

        res = graph.query(
            """
            CALL {
                MATCH (n)
                RETURN n
                ORDER BY n.v ASC
                SKIP 1
            }
            RETURN n
            """
        )

        # assert that only one record was returned
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0],
            Node(label='N', properties={'name': 'Raz', 'v': 6}))

        # TODO: Add tests for LIMIT.





    def test12_order_by(self):
        """Test the ordering of the output of the sq, and the outer query"""

        # the graph has two nodes with label `N` and prop `name` with val `Raz`
        # with the `v` property with values 2 and 5.

        res = graph.query(
            """
            CALL {
                UNWIND [1, 2, 3, 4, 5, 6, 7] AS x
                MATCH (n:N {v: x})
                RETURN n ORDER BY n.v ASC
            }
            RETURN n
            """
        )

        # validate the results
        self.env.assertEquals(len(res.result_set), 2)
        self.env.assertEquals(res.result_set[0][0],
        Node(label='N', properties={'name': 'Raz', 'v': 5}))
        self.env.assertEquals(res.result_set[1][0],
        Node(label='N', properties={'name': 'Raz', 'v': 6}))

        # TODO: These two jam the db. Why?
        # res = graph.query(
        #     """
        #     CALL {
        #         UNWIND [1, 2, 3, 4, 5, 6, 7] AS x
        #         MATCH (n:N {v: x})
        #         RETURN n ORDER BY n.v DESC
        #     }
        #     RETURN n
        #     """
        # )

        # # validate the results
        # self.env.assertEquals(len(res.result_set), 2)
        # self.env.assertEquals(res.result_set[0][0],
        # Node(label='N', properties={'name': 'Raz', 'v': 5}))
        # self.env.assertEquals(res.result_set[1][0],
        # Node(label='N', properties={'name': 'Raz', 'v': 6}))

        # res = graph.query(
        #     """
        #     CALL {
        #         UNWIND [1, 2, 3, 4, 5, 6, 7] AS x
        #         MATCH (n:N {v: x})
        #         RETURN n
        #     }
        #     RETURN n ORDER BY n.v ASC
        #     """
        # )

        # # validate the results
        # self.env.assertEquals(len(res.result_set), 2)
        # self.env.assertEquals(res.result_set[0][0],
        # Node(label='N', properties={'name': 'Raz', 'v': 1}))
        # self.env.assertEquals(res.result_set[1][0],
        # Node(label='N', properties={'name': 'Raz', 'v': 4}))



    # # TODO: Add this once UNION is supported
    # def test09_union(self):
    #     """Test that UNION works properly within a subquery"""

    #     # the graph has two nodes, with `name` 'Raz' and `v` 1 and 4

    #     res = graph.query(
    #         """
    #         CALL {
    #             MATCH (n:N)
    #             WHERE n.v = 1
    #             RETURN n
    #             UNION
    #             MATCH (n:N)
    #             WHERE n.v = 4
    #             RETURN n
    #         }
    #         RETURN n
    #         """
    #     )

    #     # assert the correctness of the results
    #     self.env.assertEquals(len(res.result_set), 2)
    #     self.env.assertEquals(res.result_set[0][0],
    #     Node(label='N', properties={'name': 'Raz', 'v': 1}))
    #     self.env.assertEquals(res.result_set[1][0],
    #     Node(label='N', properties={'name': 'Raz', 'v': 4}))


    # # TODO: Add these tests once the eager returning case is implemented
        # Note: It's supported (eager returning case) - modify the test to match new db state.
    # def test13_update(self):
    #     """Test that updates within the subquery are applied appropriately and
    #     later read correctly"""

    #     # delete the node with `v` property of value 4, so that there is only
    #     # one node in the graph, with `name`: 'Raz' and `v`: 1
    #     res = graph.query("MATCH (n:N {v: 4}) DELETE n")
    #     self.env.assertEquals(res.nodes_deleted, 1)

    #     # update the node within the sq
    #     res = graph.query(
    #         """
    #         MATCH (n:N)
    #         CALL {
    #             WITH n
    #             SET n.v = 2, n.name = 'Roi'
    #         }
    #         RETURN n
    #         """
    #     )

    #     # assert the result is correct
    #     self.env.assertEquals(len(res.result_set), 1)
    #     self.env.assertEquals(res.result_set[0][0],
    #     Node(label='N', properties={'name': 'Roi', 'v': 2}))

    #     # remove a label and a property
    #     res = graph.query(
    #         """
    #         MATCH (n:N)
    #         CALL {
    #             WITH n
    #             REMOVE n:N, n.name
    #         }
    #         RETURN n
    #         """
    #     )

    #     # assert the correctness of the effects of the query
    #     self.env.assertEquals(res.labels_removed, 1)
    #     self.env.assertEquals(res.properties_removed, 1)
    #     self.env.assertEquals(res.result_set[0][0],
    #     Node(label='', properties={}))

    #     # delete the node (?)




    # def test06_readonly(self):
    #     """~~!!Under construction!!~~"""

        # NTS: Clauses that should be tested:
        # MATCH, OPTIONAL MATCH, WHERE, ORDERBY, SKIP, LIMIT, WITH, UNION,
        # UNWIND, (partly) FOREACH
