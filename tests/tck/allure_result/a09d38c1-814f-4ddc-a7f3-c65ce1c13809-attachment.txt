MATCH (m:Movie { rating: 4 })
WITH *
MATCH (n)
RETURN toString(n.rating)