UNWIND ['.*', '', ' ', 'one'] AS strings
RETURN strings
ORDER BY strings DESC