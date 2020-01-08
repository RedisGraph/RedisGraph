MATCH p = (n)-->()
RETURN [x IN collect(p) | head(nodes(x))] AS p