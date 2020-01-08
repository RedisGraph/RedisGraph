MATCH (a)
WITH [a, 1] AS list
RETURN labels(list[1]) AS l