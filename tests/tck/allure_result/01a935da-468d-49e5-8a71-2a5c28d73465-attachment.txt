MATCH (a:A)
MATCH (a)-[:LIKES*1..2]->(c)
RETURN c.name