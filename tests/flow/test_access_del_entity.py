from common import *

class testAccessDelNode():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.g= Graph(self.env.getConnection(), "access_del_node")

    def test01_return_deleted_attribute(self):
        # try to return an attribute of a deleted entity
        # expecting an exception

        # create a node
        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        # retrieve attribute from a deleted node
        try:
            q = "MATCH (n) DELETE n RETURN n.v"
            res = self.g.query(q)
            self.env.assertEquals(res.result_set[0][0], None)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")
    
    def test02_return_deleted_node(self):
        # try to return a deleted node
        # expecting only node ID to be returned

        # create a node
        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        try:
            # return a deleted node
            q = "MATCH (n) DELETE n RETURN n"
            res = self.g.query(q)
            node = res.result_set[0][0]
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

    def _test03_deleted_node_as_argument(self):
        # try to invoke a function on a deleted node
        # expecting an exception

        # create a node
        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        # invoke function on a deleted node
        try:
            q = "MATCH (n) DELETE n RETURN labels(n)"
            res = self.g.query(q)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

    def _test04_update_deleted_node(self):
        # try to update a deleted node
        # not sure what to expect here...

        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        # update a deleted node
        q = "MATCH (n) DELETE n SET n.v = 1"
        res = self.g.query(q)

        #-----------------------------------------------------------------------

        q = "CREATE (:A {v:1}), (:B {v:2})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 2)

        # set existing node to a deleted node
        q = "MATCH (n), (m) DELETE n SET m = n"
        res = self.g.query(q)

        # clear graph
        self.env.flush()

        #-----------------------------------------------------------------------

        q = "CREATE (:A {v:1}), (:B {v:2})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 2)

        # set existing node to a deleted node
        q = "MATCH (n), (m) DELETE n SET m += n"
        res = self.g.query(q)

        # clear graph
        self.env.flush()

    def _test05_update_deleted_node_lables(self):
        # Add label to a deleted node
        # expecting an exception

        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        try:
            # add label to a deleted node
            q = "MATCH (n) DELETE n SET n:A"
            res = self.g.query(q)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

        #-----------------------------------------------------------------------

        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        try:
            # remove label from a deleted node
            q = "MATCH (n) DELETE n REMOVE n:A"
            res = self.g.query(q)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

    def _test06_merge_using_deleted_node_attr(self):
        # try to merge a node based on a deleted node attribute
        # expecting an exception
        
        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        try:
            q = "MATCH (n) DELETE n MERGE ({v:n.v})"
            res = self.g.query(q)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

    def _test07_dobule_delete(self):
        # try to delete a deleted node
        # expecting a single delete to be performed
        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        # delete a deleted node
        q = "MATCH (n) DELETE n WITH n DELETE n"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_deleted, 1)

    def _test08_create_edge_to_deleted_node(self):
        # try to create an edge to a deleted node
        # expecting an exception

        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        try:
            q = "MATCH (n) DELETE n CREATE ()-[:R]->(n)"
            res = self.g.query(q)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

        #-----------------------------------------------------------------------

        q = "CREATE (:A {v:1})"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 1)

        try:
            q = "MATCH (n) DELETE n CREATE (n)-[:R]->()"
            res = self.g.query(q)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

class testAccessDelEdge():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.g= Graph(self.env.getConnection(), "access_del_edge")

    def test01_return_deleted_attribute(self):
        # try to return an attribute of a deleted entity
        # expecting an exception

        # create an edge
        q = "CREATE ()-[:R {v:1}]->()"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 2)
        self.env.assertEquals(res.relationships_created, 1)

        # retrieve attribute from a deleted edge
        try:
            q = "MATCH ()-[e]->() DELETE e RETURN e.v"
            res = self.g.query(q)
            self.env.assertEquals(res.result_set[0][0], None)
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")
    
    def test02_return_deleted_edge(self):
        # try to return a deleted edge
        # expecting an exception

        # create an edge
        q = "CREATE ()-[:R {v:1}]->()"
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 2)
        self.env.assertEquals(res.relationships_created, 1)

        try:
            # return a deleted edge
            q = "MATCH ()-[e]->() DELETE e RETURN e"
            res = self.g.query(q)
            node = res.result_set[0][0]
            self.env.assertTrue(False)
        except Exception as e:
            self.env.assertEquals(str(e), "Entity has been deleted in this transaction")

