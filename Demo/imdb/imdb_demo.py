import os
import sys
import csv
import redis
import timeit
from client import client
from datetime import date
from disposableredis import DisposableRedis

REDIS_MODULE_PATH_ENVVAR = 'REDIS_MODULE_PATH'
REDIS_PATH_ENVVAR = 'REDIS_PATH'
REDIS_PORT_ENVVAR = 'REDIS_PORT'

graph = "imdb"
r = None
redis_graph = None

def _redis():
	module_path = os.getenv(REDIS_MODULE_PATH_ENVVAR)
	redis_path = os.getenv(REDIS_PATH_ENVVAR)
	fixed_port = os.getenv(REDIS_PORT_ENVVAR)

	if module_path is None:
		print "Undeclared environment variable {}".format(REDIS_MODULE_PATH_ENVVAR)
		print "run: export {}=../../src/libmodule.so".format(REDIS_MODULE_PATH_ENVVAR)
		return None

	if redis_path is None:
		print "Undeclared environment variable {}".format(REDIS_PATH_ENVVAR)
		print "run: export {}=<path_to_redis-server>".format(REDIS_PATH_ENVVAR)
		return None
    		
	_redis_path = redis_path
	_module_path = os.path.abspath(os.path.join(os.getcwd(), module_path))

	port = None
	if fixed_port is not None:
		port = fixed_port
	
	print "port=%s, path=%s, loadmodule=%s" % (port, redis_path, _module_path)
	dr = DisposableRedis(port=port, path=redis_path, loadmodule=_module_path)
	return dr

def PopulateGraph():
	# check if graph already exists
	if r.exists(graph) == 0:
		print "Loading movies"
		movies = LoadMovies()
		print "Loaded %d movies" % len(movies)

		print "Loading actors"
		actors = LoadActors(movies)
		print "Loaded %d actors" % len(actors)
    			
def LoadMovies():
	# Load movies entities
	movies = {}

	with open('movies.csv', 'r') as f:
		reader = csv.reader(f, delimiter=',')
		for row in reader:
			title = row[0]
			gener = row[1]
			votes = int(row[2])
			rating = float(row[3])
			year = int(row[4])
			
			movies[title] = redis_graph.create_node('title', title,
			'gener', gener,
			'votes', votes,
			'rating', rating,
			'year', year,
			label="movie")

	return movies

def LoadActors(movies):
	# Load movies entities
	actors = {}
	today = date.today()

	with open('actors.csv', 'r') as f:
		reader = csv.reader(f, delimiter=',')
		for row in reader:
			name = row[0]
			yearOfBirth = int(row[1])
			movie = row[2]
			age = today.year - yearOfBirth

			if name not in actors:
				actors[name] = redis_graph.create_node('name', name,
				'age', age, label="actor")

			if movie in movies:
				redis_graph.connect_nodes(actors[name], "act", movies[movie])

	return actors




def run_queries():
	# Query database
	#------------------------------------------------------------------------
	query_desc = "Which actors played along side Nicolas Cage?"
	print query_desc
	query = """MATCH (n:actor{name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
	RETURN a.name, m.title""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "Get 3 actors who've played along side Nicolas Cage?"
	print query_desc
	query = """MATCH (nicolas:actor {name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
	RETURN a.name, m.title
	LIMIT 3""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "Which actors played in the movie Straight Outta Compton?"
	print query_desc

	query = """MATCH (a:actor)-[:act]->(m:movie {title:"Straight Outta Compton"})
	RETURN a.name""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "Which actors who are over 50 played in blockbuster movies?"
	print query_desc

	query = """MATCH (a:actor)-[:act]->(m:movie)
	WHERE a.age >= 50 AND m.votes > 10000 AND m.rating > 8.2
	RETURN a, m"""
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "Which actors played in bad drame or comedy?"
	print query_desc

	query = """MATCH (a:actor)-[:act]->(m:movie)
	WHERE (m.gener = Drama OR m.gener = Comedy)
	AND m.rating < 6.0 AND m.votes > 80000
	RETURN a.name, m
	ORDER BY m.rating"""
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "Which young actors played along side Cameron Diaz?"
	print query_desc

	query = """MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
	WHERE a.age < 35
	RETURN a, m.title""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "Which actors played along side Cameron Diaz and are younger then her?"
	print query_desc

	query = """MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
	WHERE a.age < Cameron.age
	RETURN a, m.title""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "What's the sum and average age of the Straight Outta Compton cast?"
	print query_desc

	query = """MATCH (a:actor)-[:act]->(m:movie{title:"Straight Outta Compton"})
	RETURN m.title, SUM(a.age), AVG(a.age)""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "In how may movies did Cameron Diaz played"
	print query_desc

	query = """MATCH (Cameron:actor{name:"Cameron Diaz"})-[:act]->(m:movie)
	RETURN Cameron.name, COUNT(m.title)""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------
	query_desc = "10 Oldest actors"
	print query_desc

	query = """MATCH (a:actor)-[:act]->(m:movie)
	RETURN DISTINCT a.name, a.age
	ORDER BY a.age DESC
	LIMIT 10""";
	print "query: {query}".format(query=query)

	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

	#------------------------------------------------------------------------

def debug():
	print "debug"
	global r
	global redis_graph
	r = redis.Redis(host='localhost', port=6379)
	redis_graph = client.RedisGraph(graph, r)
	PopulateGraph()
	run_queries()

def main(argv):
	global r
	global redis_graph
	if "-debug" in argv:
		debug()
	else:
		with _redis() as r:
			redis_graph = client.RedisGraph(graph, r)
			PopulateGraph()
			run_queries()

if __name__ == '__main__':
	main(sys.argv[1:])
