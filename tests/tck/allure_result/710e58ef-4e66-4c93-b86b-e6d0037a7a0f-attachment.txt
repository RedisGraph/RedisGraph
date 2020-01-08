MATCH ()-[r1]->()-[r2]->()
RETURN type(r1), type(r2)