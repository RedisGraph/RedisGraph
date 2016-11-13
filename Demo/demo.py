import os
import csv
import operator
import timeit
from datetime import date
from disposableredis import DisposableRedis

graph = "imdb"
r = None

def redis():
	module_path = os.path.abspath(os.path.join(os.getcwd(), "../src/libmodule.so"))
	r = DisposableRedis(loadmodule = module_path)
	return r

def PopulateGraph():
	# check if graph already exists
	if r.exists(graph) == 0:
		print "Loading movies"
		movies = LoadMovies()
		print "Loaded %d movies" % len(movies)

		print "Loading actors"
		actors = LoadActors()
		print "Loaded %d actors" % len(actors)

def LoadMovies():
	# Load movies entities
	movies = []
	pipe = r.pipeline()

	with open('movies.csv', 'r') as f:
		reader = csv.reader(f, delimiter=',')
		for row in reader:
			title = row[0]
			gener = row[1]
			votes = int(row[2])
			rating = float(row[3])
			year = int(row[4])

			movies.append(title)
			pipe.hmset(title, {'title': title, 'gener': gener, 'votes': votes, 'rating': rating, 'year': year})

	pipe.execute()
	return movies

def LoadActors():
	# Load movies entities
	actors = {}
	today = date.today()
	pipe = r.pipeline()

	with open('actors.csv', 'r') as f:
		reader = csv.reader(f, delimiter=',')
		for row in reader:
			name = row[0]
			yearOfBirth = int(row[1])
			movie = row[2]
			age = today.year - yearOfBirth

			pipe.hmset(name, {'name': name, 'age': age})
			pipe.execute_command("GRAPH.ADDEDGE", graph, name, "act", movie)

			if name in actors:
				actors[name] += 1
			else:
				actors[name] = 1

	pipe.execute()
	return actors

def ExecuteQuery(query):
	print "Query: %s\n" % query

	start = timeit.default_timer()
	resultset = r.execute_command("GRAPH.QUERY", graph, query)
	elapsed = timeit.default_timer() - start

	for result in resultset:
		print "%s\n" % result

	print "Query executed in %.5f seconds\n" % elapsed

def main():
	
	global r
	with redis() as r:
		PopulateGraph()

		# Query database
		#------------------------------------------------------------------------
		print "Which actors played in the movie Straight Outta Compton?"

		query = """MATCH (actor)-[act]->(movie:"Straight Outta Compton")
		RETURN actor.name""";

		ExecuteQuery(query)

		#------------------------------------------------------------------------
		print "Which actors who are over 50 played in blockbuster movies?"

		query = """MATCH (actor)-[act]->(movie)
		WHERE actor.age >= 50 AND movie.votes > 10000 AND movie.rating > 8.5
		RETURN actor.name, actor.age, movie.title, movie.votes, movie.rating"""

		ExecuteQuery(query)

		#------------------------------------------------------------------------
		print "Which actors played in bad drame or comedy?"

		query = """MATCH (actor)-[act]->(movie)
		WHERE (movie.gener = Drama OR movie.gener = Comedy)
		AND movie.rating < 6.0 AND movie.votes > 80000
		RETURN actor.name, movie.title, movie.gener, movie.rating"""

		ExecuteQuery(query)

if __name__ == '__main__':
	main()