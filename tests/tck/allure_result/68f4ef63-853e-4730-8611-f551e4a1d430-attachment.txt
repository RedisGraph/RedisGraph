MATCH (a:Single)
OPTIONAL MATCH (a)-[*]->(b)
RETURN b