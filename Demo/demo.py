import redis
import csv

r = redis.Redis(host='localhost', port=6379)

# Load data
# Create users
with open('users.csv', 'r') as f:
	next(f) # Skip header row
	reader = csv.reader(f)
	for user in reader:
		name = user[0]
		age = user[1]
		r.hset(name, 'name', name)
		r.hset(name, 'age', age)


# Create music bands
with open('bands.csv', 'r') as f:
	next(f) # Skip header row
	reader = csv.reader(f)
	for band in reader:
		name = band[0]
		genre = band[1]
		r.hset(name, 'name', name)
		r.hset(name, 'genre', genre)


# Make connection.
graph = "music_graph"

r.execute_command("graph.ADDEDGE", graph, "Roi", "listen", "A_perfect_circle")
r.execute_command("graph.ADDEDGE", graph, "Roi", "listen", "Tool")
r.execute_command("graph.ADDEDGE", graph, "Roi", "listen", "Deftones")
r.execute_command("graph.ADDEDGE", graph, "Roi", "listen", "Gorillaz")

r.execute_command("graph.ADDEDGE", graph, "Hila", "listen", "Gorillaz")
r.execute_command("graph.ADDEDGE", graph, "Hila", "listen", "Florance_and_the_machine")

# r.execute_command("graph.ADDEDGE", graph, "Roi", "visit", "Tokyo")
# r.execute_command("graph.ADDEDGE", graph, "Roi", "visit", "California")
# r.execute_command("graph.ADDEDGE", graph, "Roi", "visit", "Tanzania")
# r.execute_command("graph.ADDEDGE", graph, "Roi", "visit", "Germany")

# r.execute_command("graph.ADDEDGE", graph, "Hila", "visit", "Germany")
# r.execute_command("graph.ADDEDGE", graph, "Hila", "visit", "Tokyo")

# Query the graph

# To which bands does Roi listens to which play either Alternative or Progressive?
query = """MATCH (me:Roi)-[listen]->(band)
	WHERE band.genre = Progressive OR band.genre = Alternative 
	RETURN me.name, me.age, band.name""";

print "Query: %s\n" % query

resultset = r.execute_command("graph.QUERY", graph, query)
print "results: %s\n" % resultset


# Who's listens to Alternative?
query = """MATCH (me)-[listen]->(band)
	WHERE band.genre = Alternative
	RETURN me.name, band.name""";

print "Query: %s\n" % query

resultset = r.execute_command("graph.QUERY", graph, query)
print "results: %s\n" % resultset

# query = "MATCH (me:Roi)-[]->(band) WHERE band.genre = Alternative RETURN me";
# r.execute_command("graph.QUERY", graph, query)