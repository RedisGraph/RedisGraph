MATCH (row)
WITH collect(row) AS rows
UNWIND rows AS node
RETURN node.id