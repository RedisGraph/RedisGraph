MATCH (n:A)
RETURN [p = (n)-[:HAS]->() | p] AS ps