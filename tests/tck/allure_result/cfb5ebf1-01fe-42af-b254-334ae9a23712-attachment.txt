MATCH (a)
MERGE (b)
WITH *
OPTIONAL MATCH (a)--(b)
RETURN count(*)