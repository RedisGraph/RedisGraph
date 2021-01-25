MATCH (a:A)
MATCH (a)-[:LIKES*2..1]->(c)
RETURN c.name