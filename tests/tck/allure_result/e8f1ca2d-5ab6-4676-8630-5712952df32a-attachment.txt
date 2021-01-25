MATCH p = (a)-[:REL*2..2]->(b:End)
RETURN relationships(p)