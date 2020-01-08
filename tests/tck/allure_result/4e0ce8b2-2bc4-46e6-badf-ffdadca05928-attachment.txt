MATCH (a)
WHERE a.name ENDS WITH '\n'
RETURN a.name AS name