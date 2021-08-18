import os
import sys
from RLTest import Env
from redis import ResponseError

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

SIGNATURE = 0
GRAPH_ID = "GraphSignature"

class testGraphSignature(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)

    # Make sure graph signature changes once a new label is created
    def test01_signature_update_on_label_creation(self):
        global SIGNATURE
        con = self.env.getConnection()

        # Adding a node without a label shouldn't update graph signature.
        q = """CREATE ()"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        # Adding a labeled node should update graph signature.
        q = """CREATE (:L)"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        q = """RETURN 1"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertTrue(isinstance(res[0], ResponseError))

        # Update signature
        SIGNATURE = int(res[1])

        # Adding a node with an existing label shouldn't update graph signature
        q = """CREATE (:L)"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        q = """RETURN 1"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

    # Make sure graph signature changes once a new relationship type is created
    def test02_signature_update_on_relation_creation(self):
        global SIGNATURE
        con = self.env.getConnection()

        # Adding edge with a new relationship type should update graph signature
        q = """CREATE ()-[:R]->()"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        q = """RETURN 1"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertTrue(isinstance(res[0], ResponseError))

        # Update signature
        SIGNATURE = int(res[1])

        # Adding edge with existing relationship type shouldn't update graph signature
        q = """CREATE ()-[:R]->()"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        q = """RETURN 1"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

    # Make sure graph signature changes once a new attribute is created
    def test03_signature_update_on_attribute_creation(self):
        global SIGNATURE
        con = self.env.getConnection()

        # Adding a new attribute should update graph signature
        q = """CREATE ({v:1})"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        q = """RETURN 1"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertTrue(isinstance(res[0], ResponseError))

        # Update signature
        SIGNATURE = int(res[1])

        # Adding a new node with existing attribute shouldn't update graph signature
        q = """CREATE ({v:1})"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        # Adding a new edge with a new attribute should update graph signature
        q = """CREATE ()-[:R {q:1}]->()"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

        q = """RETURN 1"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertTrue(isinstance(res[0], ResponseError))

        # Update signature
        SIGNATURE = int(res[1])

        # Adding a new edge with existing attribute shouldn't update graph signature
        q = """CREATE ()-[:R {v:1}]->()"""
        res = con.execute_command("GRAPH.QUERY", GRAPH_ID, q, "signature", SIGNATURE)
        self.env.assertFalse(isinstance(res[0], ResponseError))

