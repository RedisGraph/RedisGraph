UNWIND [null, 1, null] AS x
RETURN collect(DISTINCT x) AS c