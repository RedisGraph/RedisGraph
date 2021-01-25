CREATE (n:Person)-[:OWNS]->(:Dog)
RETURN labels(n)