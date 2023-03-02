from common import *
from constraint_utils import *

GRAPH_ID = "constraints"

class testConstraintNodes():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.con = self.env.getConnection()
        self.g = Graph(self.con, GRAPH_ID)
        self.populate_graph()

    def populate_graph(self):
        g = self.g
        g.query("CREATE (:Engineer:Person {name: 'Mike', age: 10, height: 180})")
        g.query("CREATE (:Engineer:Person {name: 'Tim', age: 20, height: 190})")
        g.query("CREATE (:Person:Engineer {name: 'Rick', age: 30, height: 200})")
        g.query("CREATE (:Person:Engineer {name: 'Andrew', age: 36, height: 173})")
        g.query("MATCH (a{name: 'Andrew'}),({name:'Rick'}) CREATE (a)-[:Knows {since:1984}]->(b)")

    def test01_create_constraint(self):
        #-----------------------------------------------------------------------
        # create constraints
        #-----------------------------------------------------------------------

        # create exists node constraint over Person height
        create_exists_node_constraint(self.g, 'Person', 'height')

        # create unique node constraint over Person height
        create_unique_node_constraint(self.g, 'Person', 'height')

        # create unique node constraint over Person name and age
        create_unique_node_constraint(self.g, 'Person', 'name', 'age')

        # create exists edge constraint over
        create_exists_edge_constraint(self.g, 'Knows', 'since')

        # create unique edge constraint over
        create_unique_edge_constraint(self.g, 'Knows', 'since')

        # validate constrains
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 5)
        for c in constraints:
            self.env.assertTrue(c.status != 'FAILED')

    def test02_constraint_violations(self):
        # active constrains:
        # 1. exists node constraint over Person height
        # 2. unique node constraint over Person height
        # 3. unique node constraint over Person name and age
        # 4. exists edge constraint over Knows since
        # 5. unique edge constraint over Knows since

        g = self.g

        # backup original dataset
        expected_result_set = g.query("MATCH (n) RETURN n ORDER BY ID(n)").result_set

        #-----------------------------------------------------------------------
        # create a node that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("CREATE (:Person)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, node of type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # create a node that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH (p:Person) CREATE (:Person{height:p.height})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # create a node that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH (p:Person) CREATE (:Person{height:rand(), name:p.name, age:p.age})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node update that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH (n:Person) SET n.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, node of type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # node update that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH (a:Person), (b:Person) WHERE a<>b SET a.height = b.height")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node update that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH (a:Person), (b:Person) WHERE a<>b SET a.name = b.name, a.age = b.age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-match that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (n:Person {name: 'Andrew'}) ON MATCH SET n.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, node of type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # node merge-match that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (p:Person {height:180}) ON MATCH SET p.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-match that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (p:Person {name:'Mike'}) ON MATCH SET p.age = 20, p.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-create that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (n:Person {name: 'Dor', height: 187}) ON CREATE SET n.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, node of type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # node merge-create that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (p:Person {name: 'Dor', height:187}) ON CREATE SET p.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node merge-create that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (p:Person {name:'Dor', height:187}) ON CREATE SET p.age = 10, p.name = 'Mike'")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node merge that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (n:Person {name: 'Dor'})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, node of type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # node merge that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (p:Person {name: 'Dor', height:180})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        #-----------------------------------------------------------------------
        # node merge that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE (p:Person {v:12, name:'Mike', age:10})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, node of type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # node label update which will conflict with exists constraint
        #-----------------------------------------------------------------------
        # 1. exists node constraint over Person height
        # 2. unique node constraint over Person height
        # 3. unique node constraint over Person name and age

        g.query("CREATE (:Architect)")

        #-----------------------------------------------------------------------
        # node label update which will conflict with exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH (n:Architect) SET n:Person")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, node of type Person missing attribute height", str(e))

        # add attributes to Architect which will conflict with both unique constraints
        g.query("MATCH (n:Architect) SET n.name = 'Mike', n.age = 10, n.height = 180")

        #-----------------------------------------------------------------------
        # node label update which will conflict with unique constraints
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH (n:Architect) SET n:Person")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Person", str(e))

        # delete Architect
        g.query("MATCH (n:Architect) DELETE n")

        # validate graph did not changed
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

    # TODO: add test which tries to remove index that supports a unique constraint

    def test04_invalid_constraint_command(self):
        # constraint create command:
        # GRAPH.CONSTRAIN <key> CREATE/DEL UNIQUE/EXISTS [NODE label / RELATIONSHIP type] PROPERTIES prop_count prop0...

        #-----------------------------------------------------------------------
        # invalid constraint operation
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "INVALID_OP", "unique", "LABEL", "New_Label", "PROPERTIES", 1, "New_Attr")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint operation", str(e))

        #-----------------------------------------------------------------------
        # invalid constraint type
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "CREATE", "INVALID_CT", "New_Label", "Person", "PROPERTIES", 1, "New_Attr")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint type", str(e))

        #-----------------------------------------------------------------------
        # invalid entity type
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "CREATE", "EXISTS", "INVALID_ENTITY_TYPE", "New_Label", "PROPERTIES", 1, "New_Attr")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint entity type", str(e))

        #-----------------------------------------------------------------------
        # del constraint on non exsisting label
        #-----------------------------------------------------------------------
        try:
            drop_unique_node_constraint(self.g, "None_Existing_Label", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop constraint on, no such constraint.", str(e))

        #-----------------------------------------------------------------------
        # del constraint on non exsisting attribute
        #-----------------------------------------------------------------------
        try:
            drop_unique_node_constraint(self.g, "Person", "None_Existing_Attr")
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

        # validate labels and attributes were not created for failed operations
        # not expecting None_Existing_Label, New_Label, None_Existing_Attr and New_Attr
        # to be added to the graph
        labels = self.g.query("CALL db.labels()").result_set
        attributes = self.g.query("CALL db.propertyKeys()").result_set
        self.env.assertFalse("New_Label" in labels)
        self.env.assertFalse("None_Existing_Label" in labels)
        self.env.assertFalse("New_Attr" in attributes)
        self.env.assertFalse("None_Existing_Attr" in attributes)

    def test05_constraint_create_drop_simultanously(self):
        # make sure there are no constraints in the graph
        for c in list_constraints(self.g):
            drop_constraint(self.g, c.type, c.entity_type, c.label, *c.attributes)
        self.env.assertEqual(0, len(list_constraints(self.g)))

        # create 500K new entities
        self.g.query("UNWIND range(0, 500000) AS x CREATE (:MarineBiologist {age: x})")

        # create unique constraint over MarineBiologist age attribute
        create_unique_node_constraint(self.g, "MarineBiologist", "age", sync=False)

        # make sure constraint is pending
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        c = constraints[0]
        self.env.assertEqual(c.status, "UNDER CONSTRUCTION")

        # delete constraint
        drop_unique_node_constraint(self.g, "MarineBiologist", "age")

        # constraint should be dropped immediately
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 0)

        # try to create two nodes which would have conflicted
        self.g.query("CREATE (:MarineBiologist {age: 35}), (:MarineBiologist {age: 35})")

    def test06_constraint_fix(self):
        # test that a failing constraint can be recreated successfully once
        # all conflicts are resolved

        #self.con.flushall()

        # create a Person node without any attributes
        self.g.query("CREATE (:Person)")

        # create two Person nodes with the same name
        self.g.query("CREATE (:Person {name:'jerry'})")
        self.g.query("CREATE (:Person {name:'jerry'})")

        #-----------------------------------------------------------------------
        # create a unique constraint over Person name
        #-----------------------------------------------------------------------

        create_unique_node_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation faile
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        self.env.assertEqual(constraints[0].status, "FAILED")

        # fix name uniqueness by deleting duplicated node
        self.g.query("MATCH (p:Person {name:'jerry'}) WITH p LIMIT 1 DELETE p")

        #-----------------------------------------------------------------------
        # re-create unique constraint
        #-----------------------------------------------------------------------

        create_unique_node_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation succeeded
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        self.env.assertEqual(constraints[0].status, "OPERATIONAL")

        #-----------------------------------------------------------------------
        # try to create exists constraint over Person name
        #-----------------------------------------------------------------------

        create_exists_node_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation faile
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 2)
        for c in constraints:
            if c.type == "unique":
                self.env.assertEqual(c.status, "OPERATIONAL")
            else:
                self.env.assertEqual(c.status, "FAILED")

        #-----------------------------------------------------------------------
        # try deleting a failed constraint
        #-----------------------------------------------------------------------

        drop_exists_node_constraint(self.g, "Person", "name")

        # make sure constraint was deleted
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        c = constraints[0]
        self.env.assertEqual(c.label, "Person")
        self.env.assertEqual(c.type, "unique")
        self.env.assertEqual(c.status, "OPERATIONAL")

        #-----------------------------------------------------------------------
        # re-create exists constraint
        #-----------------------------------------------------------------------

        # add missing name attribute to resolve conflict
        self.g.query("MATCH (p:Person) WHERE p.name is NULL SET p.name = 'kramer'")

        create_exists_node_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation succeeded
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 2)
        self.env.assertEqual(constraints[0].status, "OPERATIONAL")
        self.env.assertEqual(constraints[1].status, "OPERATIONAL")

    def test07_constraint_creation_with_new_label_attr(self):
        # create a constraint against a new label and a new attribute
        create_unique_node_constraint(self.g, "Artist", "nickname", sync=True)
        self.g.query("CREATE (:Artist {nickname: 'Banksy'})")

        # make sure constraint is enforced
        try:
            self.g.query("CREATE (:Artist {nickname: 'Banksy'})")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation on node of type Artist", str(e))

class testConstraintEdges():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.con = self.env.getConnection()
        self.g = Graph(self.con, GRAPH_ID)
        self.populate_graph()

    def populate_graph(self):
        g = self.g
        g.query("CREATE ()-[:Person {name: 'Mike', age: 10, height: 180}]->()")
        g.query("CREATE ()-[:Person {name: 'Tim', age: 20, height: 190}]->()")
        g.query("CREATE ()-[:Person {name: 'Rick', age: 30, height: 200}]->()")
        g.query("CREATE ()-[:Person {name: 'Andrew', age: 36, height: 173}]->()")

    def test01_create_constraint(self):
        #-----------------------------------------------------------------------
        # create constraints
        #-----------------------------------------------------------------------

        # create exists edge constraint over Person height
        create_exists_edge_constraint(self.g, 'Person', 'height')

        # create unique edge constraint over Person height
        create_unique_edge_constraint(self.g, 'Person', 'height')

        # create unique edge constraint over Person name and age
        create_unique_edge_constraint(self.g, 'Person', 'name', 'age')

        # validate constrains
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 3)
        for c in constraints:
            self.env.assertTrue(c.status != 'FAILED')

    def test02_edge_constraint_violations(self):
        # active constrains:
        # 1. exists edge constraint over Person height
        # 2. unique edge constraint over Person height
        # 3. unique edge constraint over Person name and age

        g = self.g

        # backup original dataset
        expected_result_set = g.query("MATCH ()-[e]->() RETURN e ORDER BY ID(e)").result_set

        #-----------------------------------------------------------------------
        # create an edge that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("CREATE ()-[:Person]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, edge of relationship-type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # create an edge that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH ()-[e:Person]->() CREATE ()-[:Person{height:e.height}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # create an edge that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH ()-[e:Person]->() CREATE ()-[:Person{height:rand(), name:e.name, age:e.age}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge update that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH ()-[e:Person]->() SET e.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, edge of relationship-type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # edge update that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH ()-[a:Person]->(), ()-[b:Person]->() WHERE a<>b SET a.height = b.height")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge update that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MATCH ()-[a:Person]->(), ()-[b:Person]->() WHERE a<>b SET a.name = b.name, a.age = b.age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge merge-match that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {name: 'Andrew'}]->() ON MATCH SET e.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, edge of relationship-type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # edge merge-match that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {height:180}]->() ON MATCH SET e.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge merge-match that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {name:'Mike'}]->() ON MATCH SET e.age = 20, e.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge merge-create that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {name: 'Dor', height: 187}]->() ON CREATE SET e.height = NULL")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, edge of relationship-type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # edge merge-create that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {name: 'Dor', height:187}]->() ON CREATE SET e.height = 190")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge merge-create that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {name:'Dor', height:187}]->() ON CREATE SET e.age = 10, e.name = 'Mike'")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge merge that violates the exists constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {name: 'Dor'}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, edge of relationship-type Person missing attribute height", str(e))

        #-----------------------------------------------------------------------
        # edge merge that violates the unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {name: 'Dor', height:180}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Person", str(e))

        #-----------------------------------------------------------------------
        # edge merge that violates the composite unique constraint
        #-----------------------------------------------------------------------

        try:
            g.query("MERGE ()-[e:Person {v:12, name:'Mike', age:10}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("exists constraint violation, edge of relationship-type Person missing attribute height", str(e))

        # validate graph did not changed
        actual_result_set = self.g.query("MATCH ()-[e]->() RETURN e ORDER BY ID(e)").result_set
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
        # GRAPH.CONSTRAIN <key> CREATE/DEL UNIQUE/EXISTS [NODE label / RELATIONSHIP type] PROPERTIES prop_count prop0...

        #-----------------------------------------------------------------------
        # invalid constraint operation
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "INVALID_OP", "unique", "RELATIONSHIP", "New_Label", "PROPERTIES", 1, "New_Attr")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint operation", str(e))

        #-----------------------------------------------------------------------
        # invalid constraint type
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "CREATE", "INVALID_CT", "New_Label", "Person", "PROPERTIES", 1, "New_Attr")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint type", str(e))

        #-----------------------------------------------------------------------
        # invalid entity type
        #-----------------------------------------------------------------------
        try:
            self.con.execute_command("GRAPH.CONSTRAINT", GRAPH_ID, "CREATE", "EXISTS", "INVALID_ENTITY_TYPE", "New_Label", "PROPERTIES", 1, "New_Attr")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid constraint entity type", str(e))

        #-----------------------------------------------------------------------
        # del constraint on non exsisting label
        #-----------------------------------------------------------------------
        try:
            drop_unique_edge_constraint(self.g, "None_Existing_Label", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop constraint on, no such constraint.", str(e))

        #-----------------------------------------------------------------------
        # del constraint on non exsisting attribute type
        #-----------------------------------------------------------------------
        try:
            drop_unique_edge_constraint(self.g, "Person", "None_Existing_Attr")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop constraint on, no such constraint.", str(e))

        #-----------------------------------------------------------------------
        # create constraint which already exists
        #-----------------------------------------------------------------------
        create_unique_edge_constraint(self.g, "Person", "age", sync=True)
        try:
            create_unique_edge_constraint(self.g, "Person", "age")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Constraint already exists", str(e))

        # validate labels and attributes were not created for failed operations
        # not expecting None_Existing_Label, New_Label, None_Existing_Attr and New_Attr
        # to be added to the graph
        labels = self.g.query("CALL db.labels()").result_set
        attributes = self.g.query("CALL db.propertyKeys()").result_set
        self.env.assertFalse("New_Label" in labels)
        self.env.assertFalse("None_Existing_Label" in labels)
        self.env.assertFalse("New_Attr" in attributes)
        self.env.assertFalse("None_Existing_Attr" in attributes)

    def test05_constraint_create_drop_simultanously(self):
        # make sure there are no constraints in the graph
        for c in list_constraints(self.g):
            drop_constraint(self.g, c.type, c.entity_type, c.label, *c.attributes)
        self.env.assertEqual(0, len(list_constraints(self.g)))

        # create 500K new entities
        self.g.query("UNWIND range(0, 500000) AS x CREATE ()-[:MarineBiologist {age: x}]->()")

        # create unique constraint over MarineBiologist age attribute
        create_unique_edge_constraint(self.g, "MarineBiologist", "age", sync=False)

        # make sure constraint is pending
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        c = constraints[0]
        self.env.assertEqual(c.status, "UNDER CONSTRUCTION")

        # delete constraint
        drop_unique_edge_constraint(self.g, "MarineBiologist", "age")

        # constraint should be dropped immediately
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 0)

        # try to create two edges which would have conflicted
        self.g.query("CREATE ()-[:MarineBiologist {age: 35}]->(), ()-[:MarineBiologist {age: 35}]->()")

    def test06_constraint_fix(self):
        # test that a failing constraint can be recreated successfully once
        # all conflicts are resolved

        #self.con.flushall()

        # create a Person edge without any attributes
        self.g.query("CREATE ()-[:Person]->()")

        # create two Person edgess with the same name
        self.g.query("CREATE ()-[:Person {name:'jerry'}]->()")
        self.g.query("CREATE ()-[:Person {name:'jerry'}]->()")

        #-----------------------------------------------------------------------
        # create a unique constraint over Person name
        #-----------------------------------------------------------------------

        create_unique_edge_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation faile
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        self.env.assertEqual(constraints[0].status, "FAILED")

        # fix name uniqueness by deleting duplicated edge
        self.g.query("MATCH ()-[e:Person {name:'jerry'}]->() WITH e LIMIT 1 DELETE e")

        #-----------------------------------------------------------------------
        # re-create unique constraint
        #-----------------------------------------------------------------------

        create_unique_edge_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation succeeded
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        self.env.assertEqual(constraints[0].status, "OPERATIONAL")

        #-----------------------------------------------------------------------
        # try to create exists constraint over Person name
        #-----------------------------------------------------------------------

        create_exists_edge_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation faile
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 2)
        for c in constraints:
            if c.type == "unique":
                self.env.assertEqual(c.status, "OPERATIONAL")
            else:
                self.env.assertEqual(c.status, "FAILED")

        #-----------------------------------------------------------------------
        # try deleting a failed constraint
        #-----------------------------------------------------------------------

        drop_exists_edge_constraint(self.g, "Person", "name")

        # make sure constraint was deleted
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 1)
        c = constraints[0]
        self.env.assertEqual(c.label, "Person")
        self.env.assertEqual(c.type, "unique")
        self.env.assertEqual(c.status, "OPERATIONAL")

        #-----------------------------------------------------------------------
        # re-create exists constraint
        #-----------------------------------------------------------------------

        # add missing name attribute to resolve conflict
        self.g.query("MATCH ()-[e:Person]->() WHERE e.name is NULL SET e.name = 'kramer'")

        create_exists_edge_constraint(self.g, "Person", "name", sync=True)

        # make sure constraint creation succeeded
        constraints = list_constraints(self.g)
        self.env.assertEqual(len(constraints), 2)
        self.env.assertEqual(constraints[0].status, "OPERATIONAL")
        self.env.assertEqual(constraints[1].status, "OPERATIONAL")

    def test07_constraint_creation_with_new_relation_attr(self):
        # create a constraint against a new relationship-type and a new attribute
        create_unique_edge_constraint(self.g, "Artist", "nickname", sync=True)
        self.g.query("CREATE ()-[:Artist {nickname: 'Banksy'}]->()")

        # make sure constraint is enforced
        try:
            self.g.query("CREATE ()-[:Artist {nickname: 'Banksy'}]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("unique constraint violation, on edge of relationship-type Artist", str(e))

