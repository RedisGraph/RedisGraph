MATCH (n:A)
RETURN count([p = (n)-[:HAS]->() | p]) AS c