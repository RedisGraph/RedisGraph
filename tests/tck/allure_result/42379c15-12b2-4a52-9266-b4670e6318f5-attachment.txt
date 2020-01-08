UNWIND [null, null] AS x
RETURN collect(DISTINCT x) AS c