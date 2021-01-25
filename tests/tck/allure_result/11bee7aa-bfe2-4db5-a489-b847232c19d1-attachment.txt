CREATE (m {id: 0})
WITH {first: m.id} AS m
WITH {second: m.first} AS m
RETURN m.second