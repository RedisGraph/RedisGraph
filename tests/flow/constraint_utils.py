import time
from index_utils import *

class Constraint():
    def __init__(self, ct, et, lbl, attrs, status):
        self.lbl = lbl
        self.s = status
        self.et = et
        self.attrs = attrs
        self.ct = ct

    @property
    def label(self):
        return self.lbl

    @property
    def status(self):
        return self.s

    @property
    def entity_type(self):
        return self.et

    @property
    def attributes(self):
        return self.attrs

    @property
    def type(self):
        return self.ct

# wait for constraint to fail
def wait_on_constraint_to_fail(g, ct_type, entity_type, lbl, *props):
    q = f"""CALL db.constraints() YIELD type, label, status
    WHERE type = '{t}' AND label = '{label}' AND status = 'FAILED'
    RETURN count(1)"""

    while True:
        result = g.query(q)
        if result.result_set[0][0] == 1:
            break
        time.sleep(0.5) # sleep 500ms

def wait_on_constraint(g, ct_type, entity_type, lbl, *props):
    props_filter = "properties = [" + ",".join(["'" + p + "'" for p in props]) + "]"

    q = f"""CALL db.constraints() YIELD type, label, status, properties
    WHERE type = '{ct_type}' AND label = '{lbl}' AND status <> 'OPERATIONAL'
    AND {props_filter} RETURN count(1)"""

    while True:
        result = g.query(q)
        if result.result_set[0][0] == 0:
            break
        time.sleep(0.5) # sleep 500ms

# validate constraint is being populated
def constraint_under_construction(g, ct_type, entity_type, lbl, *props):
    params = {'lbl': label, 'typ': t}
    res = g.query("CALL db.constraints() YIELD type, label, status WHERE label = $lbl AND type = $typ RETURN status", params)
    return "UNDER CONSTRUCTION" in res.result_set[0][0]

# wait for all graph constraints to by operational
def wait_for_constraints_to_sync(g):
    q = "CALL db.constraints() YIELD status WHERE status <> 'OPERATIONAL' RETURN count(1)"
    while True:
        result = g.query(q)
        if result.result_set[0][0] == 0:
            break
        time.sleep(0.5) # sleep 500ms

def create_constraint(g, ct_type, entity_type, lbl, *props, sync=False):
    args = ["GRAPH.CONSTRAINT", g.name, "CREATE", ct_type, entity_type, lbl, "PROPERTIES", len(props)]
    args.extend(props)
    res = g.execute_command(*args)
    if sync:
        wait_on_constraint(g, ct_type, entity_type, lbl, *props)

    return res

def create_unique_constraint(g, entity_type, lbl, *props, sync=False):
    return create_constraint(g, "unique", entity_type, lbl, *props, sync=sync)

def create_mandatory_constraint(g, entity_type, lbl, *props, sync=False):
    return create_constraint(g, "mandatory", entity_type, lbl, *props, sync=sync)

def create_unique_node_constraint(g, lbl, *props, sync=False):
    # create exact-match index
    create_node_exact_match_index(g, lbl, *props, sync=False)

    return create_unique_constraint(g, "LABEL", lbl, *props, sync=sync)

def create_unique_edge_constraint(g, rel, *props, sync=False):
    # create exact-match index
    create_edge_exact_match_index(g, rel, *props, sync=False)

    return create_unique_constraint(g, "RELATIONSHIP", rel, *props, sync=sync)

def create_mandatory_node_constraint(g, lbl, *props, sync=False):
    return create_mandatory_constraint(g, "LABEL", lbl, *props, sync=sync)

def create_mandatory_edge_constraint(g, lbl, *props, sync=False):
    return create_mandatory_constraint(g, "RELATIONSHIP", lbl, *props, sync=sync)

def drop_constraint(g, ct_type, entity_type, lbl, *props):
    if(entity_type == "NODE"):
        entity_type = "LABEL"

    params = ["GRAPH.CONSTRAINT", g.name, "DEL", ct_type, entity_type, lbl, "PROPERTIES", len(props)]
    params.extend(props)
    res = g.execute_command(*params)
    return res

def drop_unique_constraint(g, lbl_type, lbl, *props):
    return drop_constraint(g, "unique", lbl_type, lbl, *props)

def drop_mandatory_constraint(g, lbl_type, lbl, *props):
    return drop_constraint(g, "mandatory", lbl_type, lbl, *props)

def drop_unique_node_constraint(g, lbl, *props):
    return drop_unique_constraint(g, "LABEL", lbl, *props)

def drop_unique_edge_constraint(g, lbl, *props):
    return drop_unique_constraint(g, "RELATIONSHIP", lbl, *props)

def drop_mandatory_node_constraint(g, lbl, *props):
    return drop_mandatory_constraint(g, "LABEL", lbl, *props)

def drop_mandatory_edge_constraint(g, lbl, *props):
    return drop_mandatory_constraint(g, "RELATIONSHIP", lbl, *props)

def list_constraints(g):
    q = "CALL db.constraints() YIELD type, label, properties, entitytype, status"
    results = g.query(q).result_set

    constraints = []
    for row in results:
        t      = row[0]
        lbl    = row[1]
        attrs  = row[2]
        et     = row[3]
        status = row[4]
        constraints.append(Constraint(t, et, lbl, attrs, status))

    return constraints

