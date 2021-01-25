MATCH (n)
RETURN n.name, count(*) AS foo
ORDER BY n.name