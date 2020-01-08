MATCH (n:X)
RETURN n, size([(n)--() | 1]) > 0 AS b