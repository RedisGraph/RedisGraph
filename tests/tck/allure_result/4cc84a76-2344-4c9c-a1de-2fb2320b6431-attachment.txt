MATCH (n)
WHERE NOT(n.name = 'apa' AND false)
RETURN n