import os
import csv
import operator
import redis
from datetime import date
from query_executor import ExecuteQuery


graph = "imdb"
r = redis.Redis(host='localhost', port=6379)


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



def main():
	
	PopulateGraph()

	# Query database
	#------------------------------------------------------------------------
	qDesc = "Which actors played along side Nicolas Cage?"
	query = """MATCH (Nicolas:"Nicolas Cage")-[act]->(movie)<-[act]-(actor)
	RETURN actor.name, movie.title""";

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "Get 3 actors who've played along side Nicolas Cage?"
	query = """MATCH (Nicolas:"Nicolas Cage")-[act]->(movie)<-[act]-(actor)
	RETURN actor.name, movie.title
	LIMIT 3""";

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "Which actors played in the movie Straight Outta Compton?"

	query = """MATCH (actor)-[act]->(movie:"Straight Outta Compton")
	RETURN actor.name""";

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "Which actors who are over 50 played in blockbuster movies?"

	query = """MATCH (actor)-[act]->(movie)
	WHERE actor.age >= 50 AND movie.votes > 10000 AND movie.rating > 8.5
	RETURN actor.name, actor.age, movie.title, movie.votes, movie.rating"""

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "Which actors played in bad drame or comedy?"

	query = """MATCH (actor)-[act]->(movie)
	WHERE (movie.gener = Drama OR movie.gener = Comedy)
	AND movie.rating < 6.0 AND movie.votes > 80000
	RETURN actor.name, movie.title, movie.gener, movie.rating"""

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "Which young actors played along side Cameron Diaz?"

	query = """MATCH (Cameron:"Cameron Diaz")-[act]->(movie)<-[act]-(actor)
	WHERE actor.age < 35
	RETURN actor.name, actor.age, movie.title""";

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "Which actors played along side Cameron Diaz and are younger then her?"
		
	query = """MATCH (Cameron:"Cameron Diaz")-[act]->(movie)<-[act]-(actor)
	WHERE actor.age < Cameron.age
	RETURN actor.name, actor.age, movie.title""";

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "What's the sum and average age of the Straight Outta Compton cast?"

	query = """MATCH (actor)-[act]->(movie:"Straight Outta Compton")
	RETURN movie.title, SUM(actor.age), AVG(actor.age)""";

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "In how may movies did Cameron Diaz played"

	query = """MATCH (Cameron:"Cameron Diaz")-[act]->(movie)
	RETURN Cameron.name, COUNT(movie.title)""";

	ExecuteQuery(r, query, graph, qDesc)

	#------------------------------------------------------------------------
	qDesc = "10 Oldest actors"

	query = """MATCH (actor)-[act]->(movie)
	RETURN actor.name, actor.age
	ORDER BY actor.age
	LIMIT 10""";

	ExecuteQuery(r, query, graph, qDesc)


if __name__ == '__main__':
	main()