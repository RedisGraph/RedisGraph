MATCH p = (a:Start)-[:REL*2..2]->(b)
RETURN relationships(p)