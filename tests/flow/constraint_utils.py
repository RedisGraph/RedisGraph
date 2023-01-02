import time

def _wait_on_constraint(graph, label, t):
    q = f"""CALL db.constraints() YIELD type, label, status
    WHERE type = '{t}' AND label = '{label}' AND status <> 'OPERATIONAL'
    RETURN count(1)"""

    while True:
        result = graph.query(q)
        if result.result_set[0][0] == 0:
            break

def _create_constraint(graph, con, q, label=None, t=None, sync=False):
    res = con.execute_command(q)

    if sync:
        _wait_on_constraint(graph, label, t)

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

def create_node_unique_constraint(graph, con, key, label, *properties, sync=False):
    q = f"GRAPH.CONSTRAINT CREATE {key} UNIQUE LABEL {label} PROPERTIES {len(properties)} " + ' '.join(map('n.{0}'.format, properties))
    print(q)
    return _create_constraint(graph, con, q, label, "unique", sync)

def create_edge_unique_constraint(graph, con, key, relation, *properties, sync=False):
    q = f"GRAPH.CONSTRAINT CREATE {key} UNIQUE RELTYPE {relation} PROPERTIES {len(properties)} " + ' '.join(map('n.{0}'.format, properties))
    return _create_constraint(graph, con, q, relation, "unique", sync)

def drop_node_unique_constraint(con, key, label, *properties):
    q = f"GRAPH.CONSTRAINT DEL {key} UNIQUE LABEL {label} PROPERTIES {len(properties)} " + ' '.join(map('n.{0}'.format, properties))
    return con.execute_command(q)

def drop_edge_unique_constraint(con, key, relation, *properties):
    q = f"GRAPH.CONSTRAINT DEL {key} UNIQUE RELTYPE {relation} PROPERTIES {len(properties)} " + ' '.join(map('n.{0}'.format, properties))
    return con.execute_command(q)

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