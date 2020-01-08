MATCH p=(a:L)-[*]->(b)
RETURN b, avg(length(p))