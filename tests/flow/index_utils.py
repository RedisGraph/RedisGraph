import time

def _wait_on_index(graph, label, t):
    q = f"""CALL db.indexes() YIELD type, label, status
    WHERE type = '{t}' AND label = '{label}' AND status <> 'OPERATIONAL'
    RETURN count(1)"""

    while True:
        result = graph.query(q)
        if result.result_set[0][0] == 0:
            break

def _create_index(graph, q, label=None, t=None, sync=False):
    res = graph.query(q)

    if sync:
        _wait_on_index(graph, label, t)

    return res

def list_indicies(graph, label=None, t=None):
    q = "CALL db.indexes()"
    
    if label is not None or t is not None:
        q += " YIELD type, label, properties, language, stopwords, entitytype, info, status"
        if label is None and t is not None:
            q += f" WHERE label = '{label}' AND type = '{t}'"
        elif label is not None:
            q += f" WHERE label = '{label}'"
        elif t is not None:
            q += f" WHERE type = '{t}'"
        q += " RETURN type, label, properties, language, stopwords, entitytype, info, status"

    return graph.query(q)

def create_node_exact_match_index(graph, label, *properties, sync=False):
    q = f"CREATE INDEX for (n:{label}) on (" + ','.join(map('n.{0}'.format, properties)) + ")"
    return _create_index(graph, q, label, "exact-match", sync)

def create_edge_exact_match_index(graph, relation, *properties, sync=False):
    q = f"CREATE INDEX for ()-[r:{relation}]->() on (" + ','.join(map('r.{0}'.format, properties)) +")"
    return _create_index(graph, q, relation, "exact-match", sync)

def create_fulltext_index(graph, label, *properties, sync=False):
    q = f"CALL db.idx.fulltext.createNodeIndex('{label}', "
    q += ','.join(map("'{0}'".format, properties))
    q += ")"
    return _create_index(graph, q, label, "full-text", sync)

def drop_exact_match_index(graph, label, attribute):
    q = f"DROP INDEX ON :{label}({attribute})"
    return graph.query(q)

def drop_fulltext_index(graph, label):
    q = f"CALL db.idx.fulltext.drop('{label}')"
    return graph.query(q)

# validate index is being populated
def index_under_construction(graph, label, t):
    params = {'lbl': label, 'typ': t}
    res = graph.query("CALL db.indexes() YIELD type, label, status WHERE label = $lbl AND type = $typ RETURN status", params)
    return "UNDER CONSTRUCTION" in res.result_set[0][0]

# wait for all graph indices to by operational
def wait_for_indices_to_sync(graph):
    q = "CALL db.indexes() YIELD status WHERE status <> 'OPERATIONAL' RETURN count(1)"
    while True:
        result = graph.query(q)
        if result.result_set[0][0] == 0:
            break
        time.sleep(0.5) # sleep 500ms

