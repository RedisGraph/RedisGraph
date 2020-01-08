MATCH p = ({name: 'A'})-[:KNOWS*..2]->()
RETURN p