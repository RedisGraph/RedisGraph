UNWIND ['true', 'false'] AS s
RETURN toBoolean(s) AS b