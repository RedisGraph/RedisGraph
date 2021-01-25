MATCH (n)
REMOVE n:Bar
RETURN labels(n)