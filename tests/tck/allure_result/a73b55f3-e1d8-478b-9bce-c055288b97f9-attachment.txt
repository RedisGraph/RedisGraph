MATCH (n)
WITH [n] AS users
MATCH (users)-->(messages)
RETURN messages