import os
import csv
import operator
import timeit
from datetime import date
from disposableredis import DisposableRedis

graph = "facebook"
r = None

def redis():
	module_path = os.path.abspath(os.path.join(os.getcwd(), "../src/libmodule.so"))
	r = DisposableRedis(loadmodule = module_path)
	return r    

def createPerson(name, age, gender, status):
    return {"name":name, "age":age, "gender":gender, "status":status, "visited":[], "friendsWith":[]}

def visited(person, location):
    person["visited"].append(location)

def friendsWith(person, friend):
    person["friendsWith"].append(friend)

def PopulateGraph():
    Roi = createPerson("Roi Lipman", 32,  "male", "married")
    visited(Roi, "usa") 
    visited(Roi, "prague")
    visited(Roi, "tokyo")

    Alon = createPerson("Alon Fital", 32,  "male", "married")
    visited(Alon, "prague")
    visited(Alon, "usa")

    Ailon = createPerson("Ailon Velger", 32, "male", "married")
    visited(Alon, "greece")
    visited(Alon, "usa")

    Ori = createPerson("Ori Laslo", 32, "male", "married")
    visited(Ori, "canada")
    visited(Ori, "usa")
    visited(Ori, "china")

    Boaz = createPerson("Boaz Arad", 31, "male",  "married")
    visited(Boaz, "amsterdam")
    visited(Boaz, "usa") 

    Omri = createPerson("Omri Traub", 33, "male", "single")
    visited(Omri, "usa")
    visited(Omri, "greece")
    visited(Omri, "barcelona")
    visited(Omri, "andora")

    Tal = createPerson("Tal Doron", 32, "male", "single")
    visited(Tal, "usa")
    visited(Tal, "tokyo")
    visited(Tal, "andora")

    Lucy = createPerson("Lucy yanfital", 30, "female", "married")

    visited(Lucy, "prague")
    visited(Lucy, "usa")
    visited(Lucy, "kazakhstan")

    Jane = createPerson("Jane chernomorin", 31, "female", "married")
    visited(Jane, "usa")
    visited(Jane, "greece")

    Shelly = createPerson("Shelly laslo rooz", 31, "female", "married")
    visited(Shelly, "usa")
    visited(Shelly, "canada")
    visited(Shelly, "china")

    Valerie = createPerson("Valerie abigail arad", 31, "female", "married")
    visited(Valerie, "usa")
    visited(Valerie, "amsterdam")
    visited(Valerie, "russia")

    friendsWith(Roi, Alon)
    friendsWith(Roi, Ailon)
    friendsWith(Roi, Ori)
    friendsWith(Roi, Boaz)
    friendsWith(Roi, Omri)
    friendsWith(Roi, Tal)

    friendsWith(Alon, Lucy)
    friendsWith(Ailon, Jane)
    friendsWith(Ori, Shelly)
    friendsWith(Boaz, Valerie)

    pipe = r.pipeline()
    AddPersonToGraph(Roi, pipe)
    pipe.execute()


def AddPersonToGraph(person, pipe):
    for k in person:
        if type(person[k]) == str or type(person[k]) == int:
            pipe.hmset(person["name"], {k: person[k]})
    
    # for country in person["visited"]:
    #         pipe.execute_command("GRAPH.ADDEDGE", graph, person["name"], "visited", country)
    #         print "%s visited %s" %(person["name"], country)
    
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
        print "FOF?"
        query = """MATCH (ME:"Roi Lipman")-[friend]->()-[friend]->(fof) RETURN fof.name""";
        ExecuteQuery(query)
		
if __name__ == '__main__':
	main()