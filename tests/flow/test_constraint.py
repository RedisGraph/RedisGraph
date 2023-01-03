from common import *
from constraint_utils import *

GRAPH_ID = "G"
redis_graph = None
redis_con = None

class testConstraintFlow(FlowTestsBase):

    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph1
        global redis_con
        redis_con = self.env.getConnection()
        graph1 = Graph(redis_con, "G1")
        self.populate_graph()

    def populate_graph(self):
        global graph1
        graph1.query("CREATE (:PERSON {name: 'Mike', age: 10})")
        graph1.query("CREATE (:PERSON {name: 'Tim', age: 20})")
        graph1.query("CREATE (:PERSON {name: 'Rick', age: 30})")

    def test_constraint_creation_failiure(self):
        res = create_node_unique_constraint(graph1, "PERSON", "age", sync=True)
        assert res == 0

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age'], 'NODE', 'OPERATIONAL']]

        # create node that violates the constraint
        try:
            graph1.query("CREATE (:PERSON {name: 'Kevin', age: 10})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

