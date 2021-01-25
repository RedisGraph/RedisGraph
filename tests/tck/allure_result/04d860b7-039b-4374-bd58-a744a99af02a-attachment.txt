MATCH (a:A)
MATCH (a)-[:LIKES*0]->(c)
RETURN c.name