MATCH (a)-[r]->()
WITH [r, 1] AS list
RETURN type(list[0])