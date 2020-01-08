MATCH (a:A)
MATCH (a)-[:LIKES*0..2]->(c)
RETURN c.name