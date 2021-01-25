MATCH (a)-[:ADMIN]-(b)
WHERE a:A
RETURN a.id, b.id