MATCH (a)
OPTIONAL MATCH (a)-[r:T]->()
RETURN type(r)