MATCH (a:Blue)-[r*]->(b:Green)
RETURN count(r)