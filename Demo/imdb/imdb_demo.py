import os
import sys
import csv
import redis
import timeit
from datetime import date
from disposableredis import DisposableRedis
from redisgraph import Node, Edge, Graph

REDIS_MODULE_PATH_ENVVAR = 'REDIS_MODULE_PATH'
REDIS_PATH_ENVVAR = 'REDIS_PATH'
REDIS_PORT_ENVVAR = 'REDIS_PORT'

graph_name = "imdb"
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
	if r.exists(graph_name):
		return

	print "Loading movies"
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

			node = Node(label="movie", properties={'title': '"' + title + '"',
														  'gener': gener,
														  'votes': votes,
														  'rating': rating,
														  'year': year})
			movies[title] = node
			redis_graph.add_node(node)

	print "Loaded %d movies" % len(movies)

	print "Loading actors"
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
				node = Node(label="actor", properties={'name': '"' + name + '"', 'age': age})
				actors[name] = node
				redis_graph.add_node(node)

			if movie in movies:
				edge = Edge(actors[name], "act", movies[movie])
				redis_graph.add_edge(edge)

	print "Loaded %d actors" % len(actors)
	redis_graph.commit()

def run_query(desc, query):
	print desc
	print "query: {query}".format(query=query)
	print "Execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
	redis_graph.query(query)
	print "\n"

def run_queries():
	# Query database
	#------------------------------------------------------------------------
	query_desc = "Which actors played along side Nicolas Cage?"
	query = """MATCH (n:actor{name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
	RETURN a.name, m.title""";
	run_query(query_desc, query)

	#------------------------------------------------------------------------
	query_desc = "Get 3 actors who've played along side Nicolas Cage?"
	query = """MATCH (nicolas:actor {name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
	RETURN a.name, m.title
	LIMIT 3""";
	run_query(query_desc, query)

	#------------------------------------------------------------------------
	query_desc = "Which actors played in the movie Straight Outta Compton?"
	query = """MATCH (a:actor)-[:act]->(m:movie {title:"Straight Outta Compton"})
	RETURN a.name""";
	run_query(query_desc, query)

	#------------------------------------------------------------------------
	query_desc = "Which actors who are over 50 played in blockbuster movies?"
	query = """MATCH (a:actor)-[:act]->(m:movie)
	WHERE a.age >= 50 AND m.votes > 10000 AND m.rating > 8.2
	RETURN a, m"""
	run_query(query_desc, query)

	#------------------------------------------------------------------------
	query_desc = "Which actors played in bad drama or comedy?"
	query = """MATCH (a:actor)-[:act]->(m:movie)
	WHERE (m.gener = Drama OR m.gener = Comedy)
	AND m.rating < 5.5 AND m.votes > 50000
	RETURN a.name, m
	ORDER BY m.rating"""
	run_query(query_desc, query)

	#------------------------------------------------------------------------
	query_desc = "Which young actors played along side Cameron Diaz?"
	query = """MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
	WHERE a.age < 35
	RETURN a, m.title""";
	run_query(query_desc, query)

	#------------------------------------------------------------------------
	query_desc = "Which actors played along side Cameron Diaz and are younger then her?"
	query = """MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
	WHERE a.age < Cameron.age
	RETURN a, m.title""";
	run_query(query_desc, query)

	#------------------------------------------------------------------------
	query_desc = "What's the sum and average age of the Straight Outta Compton cast?"
	query = """MATCH (a:actor)-[:act]->(m:movie{title:"Straight Outta Compton"})
	RETURN m.title, SUM(a.age), AVG(a.age)""";
	run_query(query_desc, query)

	# ------------------------------------------------------------------------
	query_desc = "In how may movies did Cameron Diaz played"
	query = """MATCH (Cameron:actor{name:"Cameron Diaz"})-[:act]->(m:movie)
	RETURN Cameron.name, COUNT(m.title)""";
	run_query(query_desc, query)

	# ------------------------------------------------------------------------
	query_desc = "10 Oldest actors"
	query = """MATCH (a:actor)
	RETURN DISTINCT a.name, a.age
	ORDER BY a.age DESC
	LIMIT 10""";
	run_query(query_desc, query)

	#------------------------------------------------------------------------

def debug():
	print "debug"
	global r
	global redis_graph
	r = redis.Redis(host='localhost', port=6379)
	redis_graph = Graph(graph_name, r)
	PopulateGraph()
	run_queries()

def main(argv):
	global r
	global redis_graph
	if "-debug" in argv:
		debug()
	else:
		with _redis() as r:
			redis_graph = Graph(graph_name, r)
			PopulateGraph()
			run_queries()

if __name__ == '__main__':
	main(sys.argv[1:])
