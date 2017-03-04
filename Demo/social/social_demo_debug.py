import os
import csv
import timeit
import csv
from datetime import date
import redis
from query_executor import ExecuteQuery

graph = "facebook"
r = redis.Redis(host='localhost', port=6379)
    

def createPerson(name, age, gender, status):
    return {"name":name, "age":age, "gender":gender, "status":status, "visited":[], "friendsWith":[]}

def visited(person, location):
    person["visited"].append(location)

def friendsWith(person, friend):
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
        pipe.hmset(country, {"name": country})
        pipe.execute_command("GRAPH.ADDEDGE", graph, person["name"], "visited", country)
    
    for friend in person["friendsWith"]:
         pipe.execute_command("GRAPH.ADDEDGE", graph, person["name"], "friend", friend["name"])
         AddPersonToGraph(friend, pipe)




def main():

    PopulateGraph()

    # Query database
    #------------------------------------------------------------------------
    qDesc = "My friends?"
    query = """MATCH (ME:"Roi Lipman")-[friend]->(f) RETURN f.name""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

    qDesc = "Friends of friends?"
    query = """MATCH (ME:"Roi Lipman")-[friend]->()-[friend]->(fof) RETURN fof.name""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

    qDesc = "Friends of friends who are single and over 30"
    query = """MATCH (ME:"Roi Lipman")-[friend]->()-[friend]->(fof) 
                WHERE fof.status = single and fof.age > 30
                RETURN fof""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

    qDesc = "Friends of friends who visited Amsterdam and are single?"
    query = """MATCH (ME:"Roi Lipman")-[friend]->()-[friend]->(fof)-[visited]->(country:Amsterdam)
            WHERE fof.status = single
            RETURN fof.name""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

    qDesc = "Friends who've been to places I've visited"
    query = """MATCH (ME:"Roi Lipman")-[visited]->(country)<-[visited]-(f)<-[friend]-(:"Roi Lipman")
            RETURN f.name, country""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

    qDesc = "Friends who are older than me"
    query = """MATCH (ME:"Roi Lipman")-[friend]->(f)
            WHERE f.age > ME.age
            RETURN f.name, f.age""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

    qDesc = "Count for each friend how many countires he or she been to"
    query = """MATCH (ME:"Roi Lipman")-[friend]->(friend)-[visited]->(country)
            RETURN friend.name, count(country.name) AS countriesVisited
            ORDER BY countriesVisited DESC
            LIMIT 10""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

    qDesc = "Friends age statistics"
    query = """MATCH (ME:"Roi Lipman")-[friend]->(f)
            RETURN ME.name, count(f.name), sum(f.age), avg(f.age), min(f.age), max(f.age)""";
    ExecuteQuery(r, query, graph, qDesc)

    #------------------------------------------------------------------------

if __name__ == '__main__':
	main()