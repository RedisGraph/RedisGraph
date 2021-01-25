MATCH (n)
RETURN n.division, count(*)
ORDER BY count(*) DESC, n.division ASC