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
        graph1.query("CREATE (:DOG {name: 'Rick', age: 30, height: 200})")
        graph1.query("CREATE (:ENGINEER:PERSON {name: 'Mike', age: 10, height: 180})")
        graph1.query("CREATE (:ENGINEER:PERSON {name: 'Tim', age: 20, height: 190})")
        graph1.query("CREATE (:PERSON:ENGINEER {name: 'Rick', age: 30, height: 200})")
        graph1.query("CREATE (:PERSON:ENGINEER {name: 'Andrew', age: 36, height: 173})")
        graph1.query("CREATE (:COW {name: 'Rick', age: 30, height: 200})")

    def test01_syntax_error_constraint_creation_del(self):
        # create constraint on invalid operation
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "NEL", "unique", "LABEL", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("NEL", "UNIQUE", "LABEL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint operation", str(e))

        # create constraint on invalid type
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNI", "LABEL", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("CREATE", "UNI", "LABEL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint type", str(e))

        # create constraint on invalid entity type
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNIQUE", "LABOL", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("CREATE", "UNIQUE", "LABOL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint entity type", str(e))

        # del constraint on non exsisting label type
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "DEL", "unique", "LABEL", "CAT", "PROPERTIES", 1, "age")
            #graph1.constraint("DEL", "UNIQUE", "LABEL", "CAT", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Trying to delete constraint from non existing label", str(e))

        # create constraint which already exists
        redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNIQUE", "LABEL", "PERSON", "PROPERTIES", 1, "age")
        wait_on_constraint(graph1, "PERSON", "UNIQUE")
        #graph1.constraint("CREATE", "UNIQUE", "LABEL", "PERSON", "age")
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNIQUE", "LABEL", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("CREATE", "UNIQUE", "LABEL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Constraint already exists", str(e))

        # del constraint on non exsisting property name
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "DEL", "unique", "LABEL", "PERSON", "PROPERTIES", 1, "weight")
            #graph1.constraint("DEL", "UNIQUE", "LABEL", "PERSON", "weight")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Property name not found", str(e))

    def test02_constraint_creation_deletion(self):
        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age'], 'NODE', 'OPERATIONAL']]

        assert drop_node_unique_constraint(graph1, "PERSON", "age")
        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=True)

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age'], 'NODE', 'OPERATIONAL']]

        # create node that violates the constraint
        try:
            graph1.query("CREATE (:PERSON:ENGINEER {name: 'Kevin', age: 10})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        # node update that violates the constraint
        try:
            graph1.query("MATCH (n:PERSON {name: 'Andrew'}) SET n.age = 10")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        # node merge-match that violates the constraint
        try:
            graph1.query("MERGE (n:PERSON {name: 'Andrew'}) ON MATCH SET n.age = 10")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        res = graph1.query("MATCH (n:PERSON {name: 'Andrew'}) RETURN n.age")
        assert res.result_set == [[36]]

        # node merge-create that violates the constraint
        try:
            graph1.query("MERGE (n:PERSON {name: 'Bob'}) ON CREATE SET n.age = 10")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        res = graph1.query("MATCH (n:PERSON {name: 'Bob'}) RETURN n.age")
        assert res.result_set == []

        assert drop_node_unique_constraint(graph1, "PERSON", "age")

    def test03_constraint_create_drop_simultanously(self):
        assert list_constraints(graph1).result_set == []

        graph1.query("UNWIND range(0,100000) AS x CREATE (:CAT {age: x, height: x + 1})")
        assert create_node_unique_constraint(graph1, "CAT", "age", sync=False)
        assert drop_node_unique_constraint(graph1, "CAT", "age")

        res = list_constraints(graph1)
        assert res.result_set == []

    def test04_multiple_constraint_creation_deletion(self):
        assert create_node_unique_constraint(graph1, "PERSON", "age", "height", sync=True)

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL']]

        graph1.query("CREATE (:PERSON {name: 'Aharon', age: 15, height: 181})")
        graph1.query("CREATE (:PERSON {name: 'James', age: 15, height: 189})")
        graph1.query("CREATE (:PERSON {name: 'James', age: 15, height: 185})")

        # create node that violates the constraint
        try:
            graph1.query("CREATE (:ENGINEER:PERSON {name: 'Dan', age: 15, height: 189})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        assert create_node_unique_constraint(graph1, "PERSON", "height", sync=True)

        # create node that violates the 2nd constraint
        try:
            graph1.query("CREATE (:PERSON {name: 'Dan', age: 55, height: 189})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        # create node that violates the 1st constraint
        try:
            graph1.query("CREATE (:PERSON {name: 'Dan', age: 15, height: 189})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))


        # update node LABEL which will cause constraint violation
        graph1.query("CREATE (:ALIEN {name: 'Dan', age: 15, height: 189})")
        try:
            graph1.query("MATCH (n:ALIEN {name: 'Dan'}) SET n:PERSON")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        res = graph1.query("MATCH (n:ALIEN {name: 'Dan'}) RETURN labels(n)")
        assert res.result_set == [[['ALIEN']]]

        # create node that don't violate any constraint
        graph1.query("CREATE (:PERSON {name: 'Dan', age: 15, height: 191})")

    def test05_constraint_creation_failiure(self):
        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=False)
        wait_on_constraint_to_fail(graph1, "PERSON", "unique")

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'NODE', 'FAILED']]

        # check that deletion of failed constraint is done correctly
        assert drop_node_unique_constraint(graph1, "PERSON", "age")

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'NODE', 'OPERATIONAL']]

        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=False)
        wait_on_constraint_to_fail(graph1, "PERSON", "unique")

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'NODE', 'FAILED']]

        # check that recreation of constraint after removing problematic nodes is done correctly
        graph1.query("MATCH (s:PERSON {age: 15}) DELETE s")

        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=True)

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'NODE', 'OPERATIONAL']]

        graph1.query("CREATE (:PERSON {name: 'James', age: 15, height: 185})")
        # create node that violates the constraint
        try:
            graph1.query("CREATE (:PERSON {name: 'Or', age: 15, height: 191})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

    def test06_constraint_creation_with_unfimiliar_label(self):
        assert create_node_unique_constraint(graph1, "LION", "age", sync=True)
        graph1.query("CREATE (:LION {name: 'Or', age: 15})")

        try:
            graph1.query("CREATE (:LION {name: 'Dan', age: 15})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label LION", str(e))


class testConstraintFlowEdges(FlowTestsBase):

    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph1
        global redis_con
        redis_con = self.env.getConnection()
        graph1 = Graph(redis_con, "G1")
        self.populate_graph()

    def populate_graph(self):
        global graph1
        graph1.query("CREATE ()-[:DOG {name: 'Rick', age: 30, height: 200}]->()")
        graph1.query("CREATE ()-[:PERSON {name: 'Mike', age: 10, height: 180}]->()")
        graph1.query("CREATE ()-[:PERSON {name: 'Tim', age: 20, height: 190}]->()")
        graph1.query("CREATE ()-[:PERSON {name: 'Rick', age: 30, height: 200}]->()")
        graph1.query("CREATE ()-[:PERSON {name: 'Andrew', age: 36, height: 173}]->()")
        graph1.query("CREATE ()-[:COW {name: 'Rick', age: 30, height: 200}]->()")

    def test01_syntax_error_constraint_creation_del(self):
        # create constraint on invalid operation
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "NEL", "unique", "RELTYPE", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("NEL", "UNIQUE", "RELTYPE", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint operation", str(e))

        # create constraint on invalid type
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNI", "RELTYPE", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("CREATE", "UNI", "RELTYPE", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint type", str(e))

        # create constraint on invalid entity type
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNIQUE", "RELTYPO", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("CREATE", "UNIQUE", "RELTYPO", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint entity type", str(e))

        # del constraint on non exsisting label type
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "DEL", "unique", "RELTYPE", "CAT", "PROPERTIES", 1, "age")
            #graph1.constraint("DEL", "UNIQUE", "RELTYPE", "CAT", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Trying to delete constraint from non existing label", str(e))

        # create constraint which already exists
        redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNIQUE", "RELTYPE", "PERSON", "PROPERTIES", 1, "age")
        wait_on_constraint(graph1, "PERSON", "UNIQUE")
        #graph1.constraint("CREATE", "UNIQUE", "RELTYPE", "PERSON", "age")
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "CREATE", "UNIQUE", "RELTYPE", "PERSON", "PROPERTIES", 1, "age")
            #graph1.constraint("CREATE", "UNIQUE", "RELTYPE", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Constraint already exists", str(e))

        # del constraint on non exsisting property name
        try:
            redis_con.execute_command("GRAPH.CONSTRAINT", "G1", "DEL", "unique", "RELTYPE", "PERSON", "PROPERTIES", 1, "weight")
            #graph1.constraint("DEL", "UNIQUE", "RELTYPE", "PERSON", "weight")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Property name not found", str(e))

    def test02_constraint_creation_deletion(self):
        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age'], 'RELATIONSHIP', 'OPERATIONAL']]

        assert drop_edge_unique_constraint(graph1, "PERSON", "age")
        assert create_edge_unique_constraint(graph1, "PERSON", "age", sync=True)

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age'], 'RELATIONSHIP', 'OPERATIONAL']]

        # create edge that violates the constraint
        try:
            graph1.query("CREATE ()-[:PERSON {name: 'Kevin', age: 10}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        # edge update that violates the constraint
        try:
            graph1.query("MATCH ()-[n:PERSON {name: 'Andrew'}]->() SET n.age = 10")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        # edge merge-match that violates the constraint
        try:
            graph1.query("MERGE ()-[n:PERSON {name: 'Andrew'}]->() ON MATCH SET n.age = 10")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        res = graph1.query("MATCH ()-[n:PERSON {name: 'Andrew'}]->() RETURN n.age")
        assert res.result_set == [[36]]

        # edge merge-create that violates the constraint
        try:
            graph1.query("MERGE ()-[n:PERSON {name: 'Bob'}]->() ON CREATE SET n.age = 10")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        res = graph1.query("MATCH ()-[n:PERSON {name: 'Bob'}]->() RETURN n.age")
        assert res.result_set == []

        assert drop_edge_unique_constraint(graph1, "PERSON", "age")

    def test03_constraint_create_drop_simultanously(self):
        assert list_constraints(graph1).result_set == []

        graph1.query("UNWIND range(0,100000) AS x CREATE ()-[:CAT {age: x, height: x + 1}]->()")
        assert create_edge_unique_constraint(graph1, "CAT", "age", sync=False)
        assert drop_edge_unique_constraint(graph1, "CAT", "age")

        res = list_constraints(graph1)
        assert res.result_set == []

    def test04_multiple_constraint_creation_deletion(self):
        assert create_edge_unique_constraint(graph1, "PERSON", "age", "height", sync=True)

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'RELATIONSHIP', 'OPERATIONAL']]

        graph1.query("CREATE ()-[:PERSON {name: 'Aharon', age: 15, height: 181}]->()")
        graph1.query("CREATE ()-[:PERSON {name: 'James', age: 15, height: 189}]->()")
        graph1.query("CREATE ()-[:PERSON {name: 'James', age: 15, height: 185}]->()")

        # create edge that violates the constraint
        try:
            graph1.query("CREATE ()-[:PERSON {name: 'Dan', age: 15, height: 189}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        assert create_edge_unique_constraint(graph1, "PERSON", "height", sync=True)

        # create edge that violates the 2nd constraint
        try:
            graph1.query("CREATE ()-[:PERSON {name: 'Dan', age: 55, height: 189}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        # create edge that violates the 1st constraint
        try:
            graph1.query("CREATE ()-[:PERSON {name: 'Dan', age: 15, height: 189}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        # create edge that don't violate any constraint
        graph1.query("CREATE ()-[:PERSON {name: 'Dan', age: 15, height: 191}]->()")

    def test05_constraint_creation_failiure(self):
        assert create_edge_unique_constraint(graph1, "PERSON", "age", sync=False)
        wait_on_constraint_to_fail(graph1, "PERSON", "unique")

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'RELATIONSHIP', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'RELATIONSHIP', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'RELATIONSHIP', 'FAILED']]

        # check that deletion of failed constraint is done correctly
        assert drop_edge_unique_constraint(graph1, "PERSON", "age")

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'RELATIONSHIP', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'RELATIONSHIP', 'OPERATIONAL']]

        assert create_edge_unique_constraint(graph1, "PERSON", "age", sync=False)
        wait_on_constraint_to_fail(graph1, "PERSON", "unique")

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'RELATIONSHIP', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'RELATIONSHIP', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'RELATIONSHIP', 'FAILED']]

        # check that recreation of constraint after removing problematic edges is done correctly
        graph1.query("MATCH ()-[s:PERSON {age: 15}]->() DELETE s")

        assert create_edge_unique_constraint(graph1, "PERSON", "age", sync=True)

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'RELATIONSHIP', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'RELATIONSHIP', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'RELATIONSHIP', 'OPERATIONAL']]

        graph1.query("CREATE ()-[:PERSON {name: 'James', age: 15, height: 185}]->()")
        # create edge that violates the constraint
        try:
            graph1.query("CREATE ()-[:PERSON {name: 'Or', age: 15, height: 191}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

    def test06_constraint_creation_with_unfimiliar_label(self):
        assert create_edge_unique_constraint(graph1, "LION", "age", sync=True)
        graph1.query("CREATE ()-[:LION {name: 'Or', age: 15}]->()")

        try:
            graph1.query("CREATE ()-[:LION {name: 'Dan', age: 15}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label LION", str(e))