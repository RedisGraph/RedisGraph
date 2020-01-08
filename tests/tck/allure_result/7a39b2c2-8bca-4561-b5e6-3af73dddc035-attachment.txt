MATCH (a)-->()
WITH a, count(*) AS relCount
WHERE relCount > 1
RETURN a