MATCH (a)
CREATE (a)-[:KNOWS]->(b {name: missing})
RETURN b