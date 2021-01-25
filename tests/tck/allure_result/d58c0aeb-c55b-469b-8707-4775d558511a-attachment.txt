MATCH (a:A)-[:KNOWS]->(b)-->(c)
RETURN c.name