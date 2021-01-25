MATCH (a)
RETURN coalesce(a.title, a.name)