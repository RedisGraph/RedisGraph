MATCH (a:A)
MATCH (a)-[:LIKES*1..0]->(c)
RETURN c.name