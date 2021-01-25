MATCH (a:A)
OPTIONAL MATCH p = (a)-[:X]->(b)
RETURN p