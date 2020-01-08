MATCH (a)
WHERE a.name CONTAINS ' '
RETURN a.name AS name