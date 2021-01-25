MATCH (a:A), (b:B)
RETURN [p = (a)-[*]->(b) | p] AS paths