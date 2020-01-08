MATCH p = (a)-[*0..1]->(b)
RETURN a, b, length(p) AS l