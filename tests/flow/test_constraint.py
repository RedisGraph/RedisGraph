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
        graph1.query("CREATE (:PERSON {name: 'Mike', age: 10, height: 180})")
        graph1.query("CREATE (:PERSON {name: 'Tim', age: 20, height: 190})")
        graph1.query("CREATE (:PERSON {name: 'Rick', age: 30, height: 200})")
        graph1.query("CREATE (:COW {name: 'Rick', age: 30, height: 200})")

    def test01_syntax_error_constraint_creation_del(self):
        # create constraint on invalid operation
        try:
            graph1.constraint("NEL", "UNIQUE", "LABEL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            print(str(e))
            self.env.assertContains("Invalid constraint operation", str(e))

        # create constraint on invalid type
        try:
            graph1.constraint("CREATE", "UNI", "LABEL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint type", str(e))

        # create constraint on invalid entity type
        try:
            graph1.constraint("CREATE", "UNIQUE", "LABOL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint entity type", str(e))

        # del constraint on non exsisting label type
        try:
            graph1.constraint("DEL", "UNIQUE", "LABEL", "CAT", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Trying to delete constraint from non existing label", str(e))

        # create constraint which already exists
        graph1.constraint("CREATE", "UNIQUE", "LABEL", "PERSON", "age")
        try:
            graph1.constraint("CREATE", "UNIQUE", "LABEL", "PERSON", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Constraint already exists", str(e))

        # del constraint on non exsisting property name
        try:
            graph1.constraint("DEL", "UNIQUE", "LABEL", "PERSON", "weight")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Property name not found", str(e))

    def test02_constraint_creation_deletion(self):
        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age'], 'NODE', 'OPERATIONAL']]

        assert drop_node_unique_constraint(graph1, "PERSON", "age") == 0
        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=True) == 0

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age'], 'NODE', 'OPERATIONAL']]

        # create node that violates the constraint
        try:
            graph1.query("CREATE (:PERSON {name: 'Kevin', age: 10})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        assert drop_node_unique_constraint(graph1, "PERSON", "age") == 0

    def test03_constraint_create_drop_simultanously(self):
        assert list_constraints(graph1).result_set == []

        graph1.query("UNWIND range(0,100000) AS x CREATE (:CAT {age: x, height: x + 1})")
        assert create_node_unique_constraint(graph1, "CAT", "age", sync=False) == 0
        assert drop_node_unique_constraint(graph1, "CAT", "age") == 0

        res = list_constraints(graph1)
        assert res.result_set == []

    def test04_multiple_constraint_creation_deletion(self):
        assert create_node_unique_constraint(graph1, "PERSON", "age", "height", sync=True) == 0

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL']]

        graph1.query("CREATE (:PERSON {name: 'Aharon', age: 15, height: 181})")
        graph1.query("CREATE (:PERSON {name: 'James', age: 15, height: 189})")
        graph1.query("CREATE (:PERSON {name: 'James', age: 15, height: 185})")

        # create node that violates the constraint
        try:
            graph1.query("CREATE (:PERSON {name: 'Dan', age: 15, height: 189})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label PERSON", str(e))

        assert create_node_unique_constraint(graph1, "PERSON", "height", sync=True) == 0

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

        # create node that don't violate any constraint
        graph1.query("CREATE (:PERSON {name: 'Dan', age: 15, height: 191})")

    def test05_constraint_creation_failiure(self):
        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=False) == 0

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'NODE', 'FAILED']]

        # check that deletion of failed constraint is done correctly
        assert drop_node_unique_constraint(graph1, "PERSON", "age") == 0

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'NODE', 'OPERATIONAL']]

        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=False) == 0

        res = list_constraints(graph1)
        assert res.result_set == [['unique', 'PERSON', ['age', 'height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['height'], 'NODE', 'OPERATIONAL'],
        ['unique', 'PERSON', ['age'], 'NODE', 'FAILED']]

        # check that recreation of constraint after removing problematic nodes is done correctly
        graph1.query("MATCH (s:PERSON {age: 15}) DELETE s")

        assert create_node_unique_constraint(graph1, "PERSON", "age", sync=True) == 0

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
        assert create_node_unique_constraint(graph1, "LION", "age", sync=True) == 0
        graph1.query("CREATE (:LION {name: 'Or', age: 15})")

        try:
            graph1.query("CREATE (:LION {name: 'Dan', age: 15})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label LION", str(e))
