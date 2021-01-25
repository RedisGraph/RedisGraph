MATCH (n)-[rel]->(x)
WHERE n.animal = x.animal
RETURN n, x