MATCH (a)
WHERE a.name CONTAINS '\n'
RETURN a.name AS name