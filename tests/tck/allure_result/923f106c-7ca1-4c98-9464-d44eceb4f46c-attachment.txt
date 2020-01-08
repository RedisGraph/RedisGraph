MATCH p = (n {name: 'A'})-[:KNOWS*1..2]->(x)
RETURN p