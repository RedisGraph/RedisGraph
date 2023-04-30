from common import *
from collections import Counter
from collections import OrderedDict
from execution_plan_util import locate_operation
from execution_plan_util import count_operation

graph = None
GRAPH_ID = "G"

def _check_subquery_compression(plan: ExecutionPlan, operation_name: str):
    callsubquery = locate_operation(plan.structured_plan, "CallSubquery")
    if (callsubquery):
        return (count_operation(callsubquery, operation_name) == 1)
    else:
        return False

class testCallSubqueryFlow():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        global redis_con
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
            "WITH 1 AS a CALL {WITH a+1 AS b RETURN b} RETURN b",
            "WITH 1 AS a CALL {WITH a AS b RETURN b} RETURN b",
            "WITH 1 AS a CALL {WITH a LIMIT 5 RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a ORDER BY a.v RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a WHERE a > 5 RETURN a} RETURN a",
            "WITH 1 AS a CALL {WITH a SKIP 5 RETURN a} RETURN a",
            "WITH true AS a CALL {WITH NOT(a) AS b RETURN b} RETURN b",
        ]:
            self.expect_error(query,
                "WITH imports in CALL {} must be simple ('WITH a')")

        # non-valid queries within CALL {}
        for query in [
            "CALL {CREATE (n:N) MATCH (n:N) RETURN n} RETURN 1",
            "WITH 1 AS a CALL {WITH a CREATE (n:N) MATCH (n:N) RETURN n} RETURN a",
            "CALL {MATCH (n:N) CREATE (n:N2)} RETURN 1"
        ]:
            # just pass in case of an error, fail otherwise
            self.expect_error(query, "")

        # non-valid queries: import an undefined identifier
        for query in [
            """
            CALL {
                WITH a
                RETURN 1 AS one
            } 
            RETURN one
            """,
            """
            WITH a
            CALL {
                WITH a 
                RETURN a AS b
            } 
            RETURN b
            """,
        ]:
            self.expect_error(query, "a not defined")
        
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

        # outer scope variables (bound) should not be returnable from the sq
        query = "MATCH (n:N) CALL {RETURN 1 AS n} RETURN n"
        self.expect_error(query, "Variable `n` already declared in outer scope")

        # a CALL {} after an updating clause requires a separating WITH
        query = "CREATE (n:N) CALL {RETURN 1} RETURN 1"
        self.expect_error(query,
            "A WITH clause is required to introduce CALL SUBQUERY after an \
updating clause.")

        # reading query after an updating subquery requires a separating WITH
        query = "CALL {MATCH (m:M) CREATE (n:N) RETURN n} MATCH (n2:N) RETURN n2"
        self.expect_error(query,
            "A WITH clause is required to introduce MATCH after an updating clause.")

        # a query can not be terminated by a returning subquery
        query = "MATCH (n:N) CALL {WITH n CREATE (m:M {n: n.v}) RETURN m}"
        self.expect_error(query, "A query cannot conclude with a returning subquery \
(must be a RETURN clause, an update clause, a procedure call or a non-returning\
 subquery)")

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
        res = graph.query("MATCH (n:N) SET n.v = 4")
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
        # test filter using WHERE
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

        # the graph has two nodes with label `N`, with `name` 'Raz' and `v` 4
        # and 5

        res = graph.query(
            """
            CALL {
                MATCH (n:N)
                FOREACH (i in [1] |
                    SET n.v = n.v + 1
                )
                RETURN n
            }
            RETURN n
            """
            # TODO: Change RETURN line to the below line once PR fixing SORT is
            # merged
            # RETURN n ORDER BY n.v ASC
        )

        # assert the correctness of the results
        self.env.assertEquals(res.result_set[0][0],
            Node(label='N', properties={'name': 'Raz', 'v': 5}))
        self.env.assertEquals(res.result_set[1][0],
            Node(label='N', properties={'name': 'Raz', 'v': 6}))

        # Test with a non-returning subquery
        # Update properties using FOREACH in subquery
        graph.query(
            """
            CALL {
                MATCH (n:N)
                FOREACH (m in [n] |
                    MERGE (:TEMP {v: m.v})
                )
            }
            """
        )

        # assert the correctness of the results
        res = graph.query("MATCH(n:TEMP) RETURN n ORDER BY n.v ASC")
        self.env.assertEquals(len(res.result_set), 2)
        self.env.assertEquals(res.result_set[0][0],
            Node(label='TEMP', properties={'v': 5}))
        self.env.assertEquals(res.result_set[1][0],
            Node(label='TEMP', properties={'v': 6}))

        # delete the nodes with label :TEMP
        res = graph.query("MATCH (n:TEMP) DELETE n")
        self.env.assertEquals(res.nodes_deleted, 2)

    # tests that SKIP and LIMIT work properly
    def test11_skip_limit(self):
        """Test that SKIP works properly when placed inside a subquery"""

        # the graph has two nodes, with `name` 'Raz' and `v` 5 and 6

        # Test LIMIT
        res = graph.query(
            """
            CALL {
                MATCH (n:N)
                RETURN n
                ORDER BY n.v ASC
                LIMIT 1
            }
            RETURN n
            """
        )

        # assert that only one record was returned
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0],
            Node(label='N', properties={'name': 'Raz', 'v': 5}))

        # Test SKIP
        res = graph.query(
            """
            CALL {
                MATCH (n:N)
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

        # Tests SKIP and LIMIT together

        # Create 10 nodes with label :W
        res = graph.query("UNWIND range(1,10) AS i CREATE(:W {name:tostring(i), value:i})")
        self.env.assertEquals(res.nodes_created, 10)

        res = graph.query(
            """
            CALL {
                MATCH (n:W)
                RETURN n
                ORDER BY n.value ASC
                SKIP 5
                LIMIT 2
            }
            RETURN n
            """
        )

        # assert that only two records were returned
        self.env.assertEquals(len(res.result_set), 2)
        self.env.assertEquals(res.result_set[0][0],
            Node(label='W', properties={'name': '6', 'value': 6}))
        self.env.assertEquals(res.result_set[1][0],
            Node(label='W', properties={'name': '7', 'value': 7}))

        # delete the nodes with label :X created only for LIMIT tests purpose
        res = graph.query("MATCH (n:W) DELETE n")
        self.env.assertEquals(res.nodes_deleted, 10)


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

        res = graph.query(
            """
            CALL {
                UNWIND [1, 2, 3, 4, 5, 6, 7] AS x
                MATCH (n:N {v: x})
                RETURN n ORDER BY n.v DESC
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

        # multiple Sort operations, in and outside the CallSubquery op
        res = graph.query(
            """
            CALL {
                UNWIND [1, 2, 3, 4, 5, 6, 7] AS x
                MATCH (n:N {v: x})
                RETURN n ORDER BY n.v ASC
            }
            RETURN n ORDER BY n.v ASC
            """
        )

        # validate the results
        self.env.assertEquals(len(res.result_set), 2)
        self.env.assertEquals(res.result_set[0][0],
        Node(label='N', properties={'name': 'Raz', 'v': 5}))
        self.env.assertEquals(res.result_set[1][0],
        Node(label='N', properties={'name': 'Raz', 'v': 6}))

    def test13_midrun_fail(self):
        """Tests that mid-run fails are recovered correctly (free'd)"""

        query = """
        WITH 1 AS x
        CALL {
            WITH x
            RETURN x / 0 AS innerReturn
        }
        RETURN innerReturn
        """
        self.expect_error(query, "Division by zero")

    def test14_nested_call_subquery(self):
        query_to_expected_result = {
            # """
            # UNWIND [0, 1, 2] AS x 
            # CALL { 
            #     CALL {
            #         RETURN 1 AS One
            #     } 
            #     RETURN 2 AS Two
            # } RETURN 0
            # """ 
            # : [[0],[0],[0]],
            """
            UNWIND [0, 1, 2] AS x 
            CALL { 
                WITH x 
                CALL {
                    RETURN 1 AS One
                } 
                RETURN 2 AS Two
            } 
            RETURN 0
            """ 
            : [[0],[0],[0]],
            # TODO: Crash reusing variable names in nested CALL{}
            # """
            # UNWIND [0, 1, 2] AS x 
            # CALL { 
            #     CALL {
            #         RETURN 1 AS x
            #     } 
            #     RETURN max(x) AS y
            # } 
            # RETURN x
            # """ 
            # : [[0], [1], [2]],
            # TODO: Wrong results using aggregation functions in inner CALL{}
            # """
            # UNWIND [0, 1, 2] AS x 
            # CALL { 
            #     WITH x 
            #     RETURN max(x) AS y
            # } 
            # RETURN x
            # """
            # : [[0], [1], [2]],
            # """
            # UNWIND [0, 1, 2] AS x 
            # CALL { 
            #     WITH x 
            #     RETURN max(x) AS y
            # }
            # RETURN y
            # """
            # : [[0], [1], [2]],
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

    # TODO: Test failing. Fix and add.
    def test15_complex(self):
        """A more complex test, using more data and inner Call {} calls"""

        # clean db
        self.env.flush()
        graph = Graph(self.env.getConnection(), GRAPH_ID)

        # create data
        graph.query("CREATE (english: Language {name: 'English'}), (spanish: Language {name: 'Spanish'}), (french: Language {name: 'French'}), (coastal: Property {name: 'Coastal'}), (usa: Country {name: 'USA', extra_data: 'foo'}), (cali: State {name: 'California'}), (sacramento: City {name: 'Sacramento', extra_data: 'bar'}), (ny: State {name: 'New York'}), (nyc: City {name: 'New York City'}), (sacramento) - [:CITY_IN_STATE] -> (cali) - [:STATE_IN_COUNTRY] -> (usa), (cali) - [:HAS_STATE_PROP] -> (coastal), (nyc) - [:CITY_IN_STATE] -> (ny) - [:STATE_IN_COUNTRY] -> (usa), (ny) - [:HAS_STATE_PROP] -> (coastal), (nyc) - [:HAS_CITY_PROP] -> (coastal), (canada: Country {name: 'Canada'}), (ontario: State {name: 'Ontario', extra_data: 'baz'}), (toronto: City {name: 'Toronto'}), (bc: State {name: 'British Columbia'}), (victoria: City {name: 'Victoria'}), (toronto) - [:CITY_IN_STATE] -> (ontario) - [:STATE_IN_COUNTRY] -> (canada), (victoria) - [:CITY_IN_STATE] -> (bc) - [:STATE_IN_COUNTRY] -> (canada), (bc) - [:HAS_STATE_PROP] -> (coastal), (victoria) - [:HAS_CITY_PROP] -> (coastal), (canada) - [:OFFICIAL_LANGUAGE] -> (english), (canada) - [:OFFICIAL_LANGUAGE] -> (french), (mexico: Country {name: 'Mexico'}), (coahuila: State {name: 'Coahuila'}), (saltillo: City {name: 'Saltillo'}), (jalisco: State {name: 'Jalisco'}), (guadalajara: City {name: 'Guadalajara'}), (saltillo) - [:CITY_IN_STATE] -> (coahuila) - [:STATE_IN_COUNTRY] -> (mexico), (guadalajara) - [:CITY_IN_STATE] -> (jalisco) - [:STATE_IN_COUNTRY] -> (mexico), (jalisco) - [:HAS_STATE_PROP] -> (coastal), (mexico) - [:OFFICIAL_LANGUAGE] -> (spanish)")

        query = """
        MATCH (c:City)-[:CITY_IN_STATE]->(state)-[:STATE_IN_COUNTRY]->(country)-[:OFFICIAL_LANGUAGE]->({name: 'English'})
        WITH collect(distinct country) as counties
        UNWIND counties as country
        MATCH (country)-[:OFFICIAL_LANGUAGE]->(lang)
        WITH country, collect(distinct lang.name) as langs
        MATCH (country)<-[:STATE_IN_COUNTRY]->(state)
        CALL {
            WITH state
            MATCH (state)<-[:CITY_IN_STATE]-(city:City)
            CALL {
                WITH city
                RETURN {type: labels(city)[0], name: city.name} as cityDetails
            }
            WITH state, collect(cityDetails) as citiesDetails
            RETURN {type: labels(state)[0], name:state.name, cities:citiesDetails} as stateDetails
        }
        WITH country, langs, collect(stateDetails) as statesDetails
        RETURN {name: country.name, langs: langs, states: statesDetails}
        """

        res = graph.query(query)

        # assert results
        self.env.assertEquals(res.result_set[0][0], {'name': 'Canada', 'langs': ['English', 'French'], 'states': [OrderedDict([('type', 'State'), ('name', 'British Columbia'), ('cities', [OrderedDict([('type', 'City'), ('name', 'Victoria')])])]), OrderedDict([('type', 'State'), ('name', 'Ontario'), ('cities', [OrderedDict([('type', 'City'), ('name', 'Toronto')])])])]})

    def test16_rewrite_same_clauses(self):
        """Tests that rewrite same clauses works properly on clauses in a
        subquery"""

        # Test CREATE
        query = """
            CALL {
                CREATE (x:X)
                CREATE (m:M)
            }
            """
        plan = graph.explain(query)
        # make sure that CREATE is called only once
        self.env.assertTrue(_check_subquery_compression(plan, "Create"))

        # Test SET
        query = """
            CALL {
                MATCH (x:X)
                SET x.v = 3 
                SET x.t ='a'
            }"""
        plan = graph.explain(query)
        # make sure that Update is called only once
        self.env.assertTrue(_check_subquery_compression(plan, "Update"))

        # Test REMOVE
        query = """
            CALL {
                MATCH (x:X)
                REMOVE x.v
                REMOVE x.t
            }"""
        plan = graph.explain(query)
        # make sure that Update is called only once
        self.env.assertTrue(_check_subquery_compression(plan, "Update"))

        # TODO: Test failing. Fix and add.
        # # Test DELETE
        # query = """
        #     CALL {
        #         MATCH (x:X)
        #         MATCH (m:M)
        #         DELETE x
        #         DELETE m
        #     }
        #     """
        # plan = graph.explain(query)
        # # make sure that DELETE is called only once
        # self.env.assertTrue(_check_subquery_compression(plan, "Update"))

    def test17_leading_with(self):
        # valid leading WITH queries
        query_to_expected_result = {
            """
            CALL {
                WITH 1 AS b 
                RETURN b
            }
            RETURN b
            """ : [[1]],
            """
            CALL {
                WITH {} AS b 
                RETURN b
            }
            RETURN b
            """ : [[{}]],
            """
            CALL {
                WITH 'a' AS b 
                RETURN b
            } 
            RETURN b""" : [['a']],
            """
            CALL { 
                WITH ['foo', 'bar'] AS l1 
                RETURN l1
            } 
            RETURN l1
            """ : [[['foo', 'bar']]],
            """
            WITH 1 AS one
            CALL {
                WITH one, 2 as two
                RETURN one + two AS three
            }
            RETURN three
            """ : [[3]],
            """
            WITH 1 AS one, 2 as two
            CALL {
                WITH one, two
                RETURN one + two AS three
            }
            RETURN three
            """ : [[3]],
            """
            WITH 1 AS a, 5 AS b 
            CALL {
                WITH a 
                WITH a, 1 + a AS b 
                RETURN a + b AS c
            } 
            RETURN c
            """ : [[3]],
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

    def test18_returning_aggregations(self):
        """Tests that we deal properly with returning aggregations instead of
        regular projections"""

        # clear the db
        self.env.flush()
        graph = Graph(self.env.getConnection(), GRAPH_ID)

        # create a node with label N
        graph.query("CREATE (:N)")

        query = """
        MATCH (n)
        CALL {
            WITH n
            OPTIONAL MATCH (m:M)
            SET m.v = 1
            RETURN collect(n) AS cn
        }
        WITH n, cn
        UNWIND cn as ns
        RETURN ns
        """

        res = graph.query(query)

        # assert results
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0], Node(label='N'))

    # def test19_unwind_optional_match_with(self):
    #     res = graph.query (
    #         """
    #         CALL { 
    #             UNWIND ['S','T'] AS l1 
    #             OPTIONAL MATCH (n:XX) 
    #             WITH n 
    #             RETURN n
    #         } 
    #         RETURN n
    #         """
    #     )

    #     # validate the results
    #     self.env.assertEquals(len(res.result_set), 2)
    #     self.env.assertEquals(res.result_set[0][0], None)
    #     self.env.assertEquals(res.result_set[1][0], None)

    # def test20_aggregation_in_subquery(self):
    #     # Create 3 nodes for this test
    #     graph.query("UNWIND range(1, 3) AS i CREATE (n:A {v:i})")
        
    #     query_to_expected_result = {
    #         """
    #         CALL {
    #             MATCH (n:A) 
    #                 WHERE n.v % 2 = 0 
    #             WITH max(n.v) AS cn
    #             RETURN cn
    #         } 
    #         RETURN cn
    #         """ 
    #         : [[2]],
    #         """
    #         CALL {
    #             OPTIONAL MATCH (n:A) 
    #                 WHERE n.v%2=0 
    #             WITH collect(n.v) AS cn 
    #             OPTIONAL MATCH (m:A) 
    #                 WHERE m.v%2=1 
    #             WITH sum(m.v) AS cm, cn  
    #             RETURN cn, cm
    #         } 
    #         RETURN cn, cm""" 
    #         : [[[2], 4]]
    #     }
    #     for query, expected_result in query_to_expected_result.items():
    #         self.get_res_and_assertEquals(query, expected_result)

    # def test21_union(self):
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
