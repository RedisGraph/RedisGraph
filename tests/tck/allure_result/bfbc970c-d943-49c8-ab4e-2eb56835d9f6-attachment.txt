MATCH (a:A)
MATCH (a)-[:LIKES..]->(c)
RETURN c.name