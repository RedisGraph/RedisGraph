MATCH (a:X)
RETURN size([(a)-[:T|OTHER]->() | 1]) AS length