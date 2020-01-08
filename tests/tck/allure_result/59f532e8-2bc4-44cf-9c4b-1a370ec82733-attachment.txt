MATCH (a:Single), (x:C)
OPTIONAL MATCH (a)-[*]->(x)
RETURN x