UNWIND $props AS prop
MERGE (p:Person {login: prop.login})
SET p.name = prop.name
RETURN p.name, p.login