from common import *
from constraint_utils import *

GRAPH_ID = "constraints"

class testConstraintFlow(FlowTestsBase):

    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.con = self.env.getConnection()
        self.g = Graph(self.con, GRAPH_ID)
        self.populate_graph()

    def populate_graph(self):
        g = self.g
        g.query("CREATE (:Engineer:Person {name: 'Mike', age: 10, height: 180})")
        g.query("CREATE (:Engineer:Person {name: 'Tim', age: 20, height: 190})")
        g.query("CREATE (:Person:engineer {name: 'Rick', age: 30, height: 200})")
        g.query("CREATE (:Person:engineer {name: 'Andrew', age: 36, height: 173})")
        g.query("MATCH (a{name: 'Andrew'}),({name:'Rick'}) CREATE (a)-[:Knows {since:1984}]->(b)")

    def test01_create_constraint(self):
        #-----------------------------------------------------------------------
        # create constraints
        #-----------------------------------------------------------------------

        # create mandatory node constraint over Person height
        create_mandatory_node_constraint(self.g, 'Person', 'height')

        # create unique node constraint over Person height
        create_unique_node_constraint(self.g, 'Person', 'height')

        # create unique node constraint over Person name and age
        create_unique_node_constraint(self.g, 'Person', 'name', 'age')

        # create mandatory edge constraint over
        create_mandatory_edge_constraint(self.g, 'Knows', 'since')

        # create unique edge constraint over
        create_unique_edge_constraint(self.g, 'Knows', 'since')

        # validate constrains
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 5)
        for c in constraints:
            self.env.assertTrue(c.status != 'FAILED')

    def test02_constraint_violations(self):
        # active constrains:
        # 1. mandatory node constraint over Person height
        # 2. unique node constraint over Person height
        # 3. unique node constraint over Person name and age

        # backup original dataset
        expected_result_set = self.g.query("MATCH (n) RETURN n ORDER BY ID(n)").result_set

        #-----------------------------------------------------------------------
        # create a node that violates the mandatory constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("CREATE (:Person)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # create a node that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MATCH (p:Person) CREATE (:Person{height:p.height})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # create a node that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MATCH (p:Person) CREATE (:Person{height:rand(), name:p.name, age:p.age})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node update that violates the mandatory constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MATCH (n:Person) SET n.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node update that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MATCH (a:Person), (b:Person) WHERE a<>b SET a.height = b.height")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node update that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MATCH (a:Person), (b:Person) WHERE a<>b SET a.name = b.name, a.age = b.age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-match that violates the mandatory constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (n:Person {name: 'Andrew'}) ON MATCH SET n.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-match that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (p:Person {height:180}) ON MATCH SET p.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-match that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (p:Person {name:'Mike'}) ON MATCH SET p.age = 20, p.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-create that violates the mandatory constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (n:Person {name: 'Dor', height: 187}) ON CREATE SET n.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-create that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (p:Person {name: 'Dor', height:187}) ON CREATE SET p.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-create that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (p:Person {name:'Dor', height:187}) ON CREATE SET p.age = 10, p.name = 'Mike'")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge that violates the mandatory constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (n:Person {name: 'Dor'})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (p:Person {name: 'Dor', height:180})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        #-----------------------------------------------------------------------
        # node merge that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            self.g.query("MERGE (p:Person {v:12, name:'Mike', age:10})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Person", str(e))

        actual_result_set = self.g.query("MATCH (n) RETURN n ORDER BY ID(n)").result_set
        self.env.assertEqual(actual_result_set, expected_result_set)

    def test03_drop_constraint(self):
        #-----------------------------------------------------------------------
        # drop constraints
        #-----------------------------------------------------------------------

        # get all constraints
        constraints = list_constraints(self.g)

        # drop each constraint
        for c in constraints:
            drop_constraint(self.g, c.type, c.entity_type, c.label, *c.attributes)

        # validate graph doesn't contains any constraints
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 0)

    def test04_invalid_constraint_command(self):
        # constraint create command:
        # GRAPH.CONSTRAIN <key> CREATE/DEL UNIQUE/MANDATORY LABEL/RELATIONSHIP label PROPERTIES prop_count prop0...

        #-----------------------------------------------------------------------
        # invalid constraint operation
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "INVALID_OP", "unique", "LABEL", "Person", "PROPERTIES", 1, "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint operation", str(e))

        #-----------------------------------------------------------------------
        # invalid constraint type
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "CREATE", "INVALID_CT", "LABEL", "Person", "PROPERTIES", 1, "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint type", str(e))

        #-----------------------------------------------------------------------
        # invalid entity type
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "CREATE", "MANDATORY", "INVALID_ENTITY_TYPE", "Person", "PROPERTIES", 1, "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint entity type", str(e))

        #-----------------------------------------------------------------------
        # del constraint on non exsisting label
        #-----------------------------------------------------------------------
        try:
            drop_unique_node_constraint(self.g, "NONE_EXISTING_LABEL", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop constraint on, no such constraint.", str(e))

        #-----------------------------------------------------------------------
        # del constraint on non exsisting attribute type
        #-----------------------------------------------------------------------
        try:
            drop_unique_node_constraint(self.g, "Person", "birthdate")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop constraint on, no such constraint.", str(e))

        #-----------------------------------------------------------------------
        # create constraint which already exists
        #-----------------------------------------------------------------------
        create_unique_node_constraint(self.g, "Person", "age", sync=True)
        try:
            create_unique_node_constraint(self.g, "Person", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Constraint already exists", str(e))

    def test05_constraint_create_drop_simultanously(self):
        # make sure there are no constraints in the graph
        for c in list_constraints(self.g):
            drop_constraint(self.g, c.type, c.entity_type, c.label, *c.attributes)
        self.env.assertEqual(0, len(list_constraints(self.g)))

        # create 500K new entities
        self.g.query("UNWIND range(0, 500000) AS x CREATE (:Cat {age: x})")

        # create unique constraint over Cat age attribute
        create_unique_node_constraint(self.g, "Cat", "age", sync=False)

        # make sure constraint is pending
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        c = constraints[0]
        self.env.assertEqual(c.status, "UNDER CONSTRUCTION")

        # delete constraint
        drop_unique_node_constraint(self.g, "Cat", "age")

        # constraint should be dropped immediately
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 0)

    def est06_multiple_constraint_creation_deletion(self):
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

    def est07_constraint_creation_failiure(self):
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

    def test08_constraint_creation_with_new_label_attr(self):
        # create a constraint against a new label and a new attribute
        create_unique_node_constraint(self.g, "Artist", "nickname", sync=True)
        self.g.query("CREATE (:Artist {nickname: 'Banksy'})")

        # make sure constraint is enforced
        try:
            self.g.query("CREATE (:Artist {nickname: 'Banksy'})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label Artist", str(e))

class estConstraintFlowEdges(FlowTestsBase):

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

    def est01_syntax_error_constraint_creation_del(self):
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

    def est02_constraint_creation_deletion(self):
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

    def est03_constraint_create_drop_simultanously(self):
        assert list_constraints(graph1).result_set == []

        graph1.query("UNWIND range(0,100000) AS x CREATE ()-[:CAT {age: x, height: x + 1}]->()")
        assert create_edge_unique_constraint(graph1, "CAT", "age", sync=False)
        assert drop_edge_unique_constraint(graph1, "CAT", "age")

        res = list_constraints(graph1)
        assert res.result_set == []

    def est04_multiple_constraint_creation_deletion(self):
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

    def est05_constraint_creation_failiure(self):
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

    def est06_constraint_creation_with_unfimiliar_label(self):
        assert create_edge_unique_constraint(graph1, "LION", "age", sync=True)
        graph1.query("CREATE ()-[:LION {name: 'Or', age: 15}]->()")

        try:
            graph1.query("CREATE ()-[:LION {name: 'Dan', age: 15}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("constraint violation on label LION", str(e))

