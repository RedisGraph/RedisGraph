from common import *

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
        graph1.query("CREATE (:PARENT {name: 'Stevie'})")
        graph1.query("CREATE (:PARENT {name: 'Mike'})")
        graph1.query("CREATE (:PARENT {name: 'James'})")
        graph1.query("CREATE (:PARENT {name: 'Rich', fname: 'Michael'})")
        graph1.query("CREATE (:PARENT {name: 'Rich', fname: 'Thomas'})")
        graph1.query("MATCH (p:PARENT {name: 'Stevie'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child1'})")
        graph1.query("MATCH (p:PARENT {name: 'Stevie'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child2'})")
        graph1.query("MATCH (p:PARENT {name: 'Stevie'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child3'})")
        graph1.query("MATCH (p:PARENT {name: 'Mike'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child4'})")
        graph1.query("MATCH (p:PARENT {name: 'James'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child5'})")
        graph1.query("MATCH (p:PARENT {name: 'James'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child6'})")

    def test_constraint_creation_failiure(self):
        res = redis_con.execute_command("GRAPH.CONSTRAINT CREATE UNIQUE LABEL PARENT PROPERTIES 1 name")
        assert res == [1]

        res = graph1.QUERY(CALL db.constraints)
        #assert res.result_set[0][0] == "UNIQUE PARENT.name"
