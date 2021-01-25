MATCH (a:A)
MATCH (a)-[:LIKES*2..2]->(c)
RETURN c.name