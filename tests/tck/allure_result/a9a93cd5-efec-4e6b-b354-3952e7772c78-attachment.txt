MATCH (node)-[r:KNOWS]->(a)
WHERE r.name = 'monkey'
RETURN a