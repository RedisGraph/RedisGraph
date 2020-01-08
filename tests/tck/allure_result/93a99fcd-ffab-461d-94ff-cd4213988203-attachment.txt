MATCH (n)
RETURN n
ORDER BY n.name ASC
SKIP $skipAmount