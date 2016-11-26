import os
import csv
import operator
import timeit
import csv
from datetime import date
from disposableredis import DisposableRedis

graph = "facebook"
r = None

def redis():
	module_path = os.path.abspath(os.path.join(os.getcwd(), "../../src/libmodule.so"))
	r = DisposableRedis(loadmodule = module_path)
	return r    


def createPerson(name, age, gender, status):
    print "Creating person: %s age: %d gender: %s status: %s" % (name, age, gender, status)
    return {"name":name, "age":age, "gender":gender, "status":status, "visited":[], "friendsWith":[]}

def visited(person, location):
    print "%s visited: %s" %(person["name"], location)
    person["visited"].append(location)

def friendsWith(person, friend):
    print "%s friends with %s" % (person["name"], friend["name"])
    person["friendsWith"].append(friend)

def PopulateGraph():
    persons = {}

    with open('person.csv', 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:
            name = row[0]
            age = int(row[1])
            gender = row[2]
            status = row[3]
            persons[name] = createPerson(name, age, gender, status)
    
    with open('visits.csv', 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:
            person = persons[row[0]]
            country = row[1]
            visited(person, country)

    with open('friends.csv', 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:        
            person = persons[row[0]]
            friend = persons[row[1]]
            friendsWith(person, friend)

    pipe = r.pipeline()
    
    for p in persons:
        AddPersonToGraph(persons[p], pipe)
    
    pipe.execute()


def AddPersonToGraph(person, pipe):
    for prop in person:
        if type(person[prop]) == str or type(person[prop]) == int:
            key = person["name"]
            pipe.hmset(key, {prop: person[prop]})
    
    for country in person["visited"]:
            pipe.execute_command("GRAPH.ADDEDGE", graph, person["name"], "visited", country)
    
    for friend in person["friendsWith"]:
         pipe.execute_command("GRAPH.ADDEDGE", graph, person["name"], "friend", friend["name"])
         AddPersonToGraph(friend, pipe)


def ExecuteQuery(query):
    print "Query: %s\n" % query
    start = timeit.default_timer()
    resultset = r.execute_command("GRAPH.QUERY", graph, query)
    elapsed = timeit.default_timer() - start
    elapsedMS = elapsed * 1000

    for result in resultset:
        print "%s\n" % result
    
    print "Query executed in %.5f miliseconds\n" % elapsedMS



def main():
    global r
    with redis() as r:
        PopulateGraph()
    
        # Query database
        #------------------------------------------------------------------------
        print "Friends of friends?"
        query = """MATCH (ME:"Roi Lipman")-[friend]->()-[friend]->(fof) RETURN fof.name""";
        ExecuteQuery(query)

        print "Friends of friends who are single and over 30"
        query = """MATCH (ME:"Roi Lipman")-[friend]->()-[friend]->(fof) 
                    WHERE fof.status = single and fof.age > 30
                    RETURN fof.name, fof.age""";
        ExecuteQuery(query)

        print "Friends of friends who visited Amsterdam and are single?"
        query = """MATCH (ME:"Roi Lipman")-[friend]->()-[friend]->(fof)-[visited]->(country:Amsterdam)
                WHERE fof.status = single
                RETURN fof.name""";
        ExecuteQuery(query)
		
if __name__ == '__main__':
	main()