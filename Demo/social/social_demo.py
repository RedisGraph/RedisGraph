import os
import csv
import sys
import redis
import timeit
from datetime import date
from client import client
from disposableredis import DisposableRedis

REDIS_MODULE_PATH_ENVVAR = 'REDIS_MODULE_PATH'
REDIS_PATH_ENVVAR = 'REDIS_PATH'
REDIS_PORT_ENVVAR = 'REDIS_PORT'

graph = "facebook"
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
    if r.exists(graph):
        return

    # dictionary person name to its node entity
    persons = {}
    # dictionary country name to its node entity
    countries = {}

    # Create country entities
    with open('countries.csv', 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:
            name = row[0]
            countries[name] = redis_graph.create_node("name", name, label="country")

    # Create person entities
    with open('person.csv', 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:
            name = row[0]
            age = int(row[1])
            gender = row[2]
            status = row[3]
            persons[name] = redis_graph.create_node("name", name,
                                                    "age", age,
                                                    "gender", gender,
                                                    "status", status,
                                                    label="person")

    # Connect people to places they've visited.
    with open('visits.csv', 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:
            person = row[0]
            country = row[1]
            purpose = row[2]
            redis_graph.connect_nodes(persons[person], "visited", countries[country], "purpose", purpose)

    # Connect friends
    with open('friends.csv', 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:
            person = persons[row[0]]
            friend = persons[row[1]]
            redis_graph.connect_nodes(person, "friend", friend)

def AddPersonToGraph(person, countries):
    print "Adding person {name} age {age} gender {gender} status {status}".format(name=person["name"],
    age=person["age"],
    gender=person["gender"],
    status=person["status"])

    person_node = redis_graph.create_node("name", person["name"],
    "age", person["age"],
    "gender", person["gender"],
    "status", person["status"], label="person")

    for country in person["visited"]:
        country_node = countries[country]
        redis_graph.connect_nodes(person_node, "visited", country_node)


def run_queries():
    print "Querying...\n"
    # Query database
    #------------------------------------------------------------------------

    query_desc = "My friends?"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person) RETURN f.name"""
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "Friends of friends?"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->(fof:person) RETURN fof.name""";
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "Friends of friends who are single and over 30"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->(fof:person {status:single}) 
                WHERE fof.age > 30
                RETURN fof""";
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "Friends of friends who visited Amsterdam and are single?"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->(fof:person {status:single})-[:visited]->(:country {name:Amsterdam})
            RETURN fof.name""";
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    # TODO: realize that both start points are the same node.
    query_desc = "Friends who've been to places I've visited"
    print query_desc
    query = """MATCH (:person {name:"Roi Lipman"})-[:visited]->(c:country)<-[:visited]-(f:person)<-[:friend]-(:person {name:"Roi Lipman"})
            RETURN f.name, c""";
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "Friends who are older than me"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
            WHERE f.age > ME.age
            RETURN f.name, f.age""";

    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    # ------------------------------------------------------------------------

    query_desc = "Count for each friend how many countires he or she been to"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(friend:person)-[:visited]->(c:country)
            RETURN friend.name, count(c.name) AS countriesVisited
            ORDER BY countriesVisited DESC
            LIMIT 10""";
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "Friends age statistics"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
            RETURN ME.name, count(f.name), sum(f.age), avg(f.age), min(f.age), max(f.age)""";
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "For each country i've been to, what was the visit purpose?"
    print query_desc
    query = """MATCH (ME:person {name:"Roi Lipman"})-[v:visited]->(c:country) RETURN c.name, v.purpose"""
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "Find out who went on a business trip"
    print query_desc
    query = """MATCH (p:person)-[v:visited {purpose:business}]->(c:country) RETURN p.name, v.purpose, c.name"""
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

    query_desc = "Count number of vacations per person"
    print query_desc
    query = """MATCH (p:person)-[v:visited {purpose:pleasure}]->(c:country)
    RETURN p.name, count(v.purpose) AS vacations
    ORDER BY vacations DESC
    LIMIT 6"""
    print "query: {query}".format(query=query)
    print "execution plan:\n{plan}".format(plan=redis_graph.execution_plan(query))
    redis_graph.query(query)
    print "\n"

    #------------------------------------------------------------------------

def debug():
    print "debug"
    global r
    global redis_graph
    r = redis.Redis(host='localhost', port=6379)
    redis_graph = client.RedisGraph(graph, r)
    print "PopulateGraph"
    PopulateGraph()
    print "run_queries"
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
