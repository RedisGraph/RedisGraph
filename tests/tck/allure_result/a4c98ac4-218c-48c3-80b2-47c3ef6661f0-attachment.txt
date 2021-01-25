MATCH (n)
REMOVE n:Foo
RETURN labels(n)