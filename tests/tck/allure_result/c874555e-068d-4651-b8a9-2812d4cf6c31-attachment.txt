MATCH (a:X)
RETURN size([(a)-[:T]->() | 1]) AS length