import time

def _wait_on_constraint(graph, label, t):
    q = f"""CALL db.constraints() YIELD type, label, status
    WHERE type = '{t}' AND label = '{label}' AND status <> 'OPERATIONAL'
    RETURN count(1)"""

    while True:
        result = graph.query(q)
        if result.result_set[0][0] == 0:
            break

def _create_constraint(graph, ct_type, entity_type, label, *properties, sync=False):
    res = graph.constraint("CREATE", ct_type, entity_type, label, *properties)

    if sync:
        _wait_on_constraint(graph, label, ct_type)

    return res

def list_constraints(graph, label=None, t=None):
    q = "CALL db.constraints()"
    
    if label is not None or t is not None:
        q += " YIELD type, label, properties, entitytype, info, status"
        if label is None and t is not None:
            q += f" WHERE label = '{label}' AND type = '{t}'"
        elif label is not None:
            q += f" WHERE label = '{label}'"
        elif t is not None:
            q += f" WHERE type = '{t}'"
        q += " RETURN type, label, properties, entitytype, info, status"

    return graph.query(q)

def create_node_unique_constraint(graph, label, *properties, sync=False):
    return _create_constraint(graph, "UNIQUE", "LABEL", label, *properties, sync=sync)

def create_edge_unique_constraint(graph, relation, *properties, sync=False):
    return _create_constraint(graph, "UNIQUE", "RELTYPE", relation, properties, sync=sync)

def drop_node_unique_constraint(graph, label, *properties):
    return graph.constraint("DEL", "UNIQUE", "LABEL", label, properties)

def drop_edge_unique_constraint(graph, relation, *properties):
    return graph.constraint("DEL", "UNIQUE", "RELTYPE", relation, properties)

# validate constraint is being populated
def constraint_under_construction(graph, label, t):
    params = {'lbl': label, 'typ': t}
    res = graph.query("CALL db.constraints() YIELD type, label, status WHERE label = $lbl AND type = $typ RETURN status", params)
    return "UNDER CONSTRUCTION" in res.result_set[0][0]

# wait for all graph constraints to by operational
def wait_for_constraints_to_sync(graph):
    q = "CALL db.constraints() YIELD status WHERE status <> 'OPERATIONAL' RETURN count(1)"
    while True:
        result = graph.query(q)
        if result.result_set[0][0] == 0:
            break
        time.sleep(0.5) # sleep 500ms