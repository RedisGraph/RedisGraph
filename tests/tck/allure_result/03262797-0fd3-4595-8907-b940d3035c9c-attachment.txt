MATCH ()-[r*0..1]-()
RETURN last(r) AS l