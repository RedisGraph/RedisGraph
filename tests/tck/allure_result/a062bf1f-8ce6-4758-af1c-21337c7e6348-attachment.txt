MATCH (a:Single)
OPTIONAL MATCH (a)-[*3..]-(b)
RETURN b