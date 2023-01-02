from common import *
from constraint_utils import *

GRAPH_ID = "G"
redis_graph = None
redis_con = None

class testConstraintFlow(FlowTestsBase):

    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph1
        redis_con = self.env.getConnection()
        graph1 = Graph(redis_con, "G1")
        self.populate_graph()


    def populate_graph(self):
        global graph1
        graph1.query("CREATE (:PERSON {name: 'Mike', age: 10})")
        graph1.query("CREATE (:PERSON {name: 'Kevin', age: 10})")
        graph1.query("CREATE (:PERSON {name: 'Tim', age: 20})")
        graph1.query("CREATE (:PERSON {name: 'Rick', age: 30})")

    def test_constraint_creation_failiure(self):
        res = create_node_unique_constraint(graph1, redis_con, "G1", "PERSON", "age")
        assert res == [0]

        res = list_constraints(graph1)
        print(res)
        #assert res.result_set[0][0] == "UNIQUE PARENT.name"
