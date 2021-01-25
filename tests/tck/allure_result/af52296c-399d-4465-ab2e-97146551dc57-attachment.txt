MATCH ()-[r]->()
DELETE r
RETURN type(r)