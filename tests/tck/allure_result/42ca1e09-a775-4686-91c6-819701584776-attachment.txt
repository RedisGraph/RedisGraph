UNWIND [true, false] AS b
RETURN toBoolean(b) AS b