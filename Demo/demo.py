import csv
import redis
import operator
from datetime import date

r = redis.Redis(host='localhost', port=6379)
graph = "imdb"


def PopulateGraph():
	# check if graph already exists
	if r.exists(graph) == 0:
		print "Loading movies"
		movies = LoadMovies()
		print "Loaded %d movies" % len(movies)

		print "Loading actors"
		actors = LoadActors()
		print "Loaded %d actors" % len(actors)

		# print "Number of actors which played in more then 2 movies %d" % len([x for x in actors if actors[x] > 2]) 

def LoadMovies():
	# Load movies entities
	movies = []
	with open('movies.csv', 'r') as f:
		reader = csv.reader(f, delimiter=',')
		for row in reader:
			# Doctor Strange,Action,71532,8.0,2016
			title = row[0]
			gener = row[1]
			votes = int(row[2])
			rating = float(row[3])
			year = int(row[4])

			r.hset(title, 'title', title)
			r.hset(title, 'gener', gener)
			r.hset(title, 'votes', votes)
			r.hset(title, 'rating', rating)
			r.hset(title, 'year', year)
			movies.append(title)

	return movies

def LoadActors():
	# Load movies entities
	actors = {}
	today = date.today()

	with open('actors.csv', 'r') as f:
		reader = csv.reader(f, delimiter=',')
		for row in reader:
			# Chris Pratt,1979,Guardians of the Galaxy
			name = row[0]
			yearOfBirth = int(row[1])
			movie = row[2]
			age = today.year - yearOfBirth

			r.hset(name, 'name', name)
			r.hset(name, 'age', age)			
			r.execute_command("GRAPH.ADDEDGE", graph, name, "act", movie)

			if name in actors:
				actors[name] += 1
			else:
				actors[name] = 1

		
	# topActors = sorted(actors.items(), key=operator.itemgetter(1))
	# print "top 10 actors %s" % (topActors[-10:])
	return actors

def ExecuteQuery(query):
	print "Query: %s\n" % query

	resultset = r.execute_command("GRAPH.QUERY", graph, query)
	print "results: %s\n" % resultset

def main():
	
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