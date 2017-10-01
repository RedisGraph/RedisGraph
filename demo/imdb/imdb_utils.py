import csv
import os
from datetime import date
from redisgraph import Node, Edge


graph_name = "imdb"


def populate_graph(redis_con, redis_graph):
	# check if graph already exists
	if redis_con.exists(graph_name):
		return

	# Load movies entities
	print("Loading movies")
	movies = {}

	with open(os.path.dirname(os.path.abspath(__file__)) + '/resources/movies.csv', 'r') as f:
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

	print("Loaded %d movies" % len(movies))

	# Load actors entities
	print("Loading actors")
	actors = {}
	today = date.today()

	with open(os.path.dirname(os.path.abspath(__file__)) + '/resources/actors.csv', 'r') as f:
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

	print("Loaded %d actors" % len(actors))
	redis_graph.commit()
