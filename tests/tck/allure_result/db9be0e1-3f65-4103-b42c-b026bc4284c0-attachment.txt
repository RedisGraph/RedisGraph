MATCH (n)
WITH n LIMIT toInteger(ceil(1.7))
RETURN count(*) AS count