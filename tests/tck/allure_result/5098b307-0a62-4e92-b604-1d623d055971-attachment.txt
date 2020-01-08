MATCH (a)
WHERE a.name STARTS WITH '\n'
RETURN a.name AS name