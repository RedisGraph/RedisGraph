import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

graph_entities = QueryInfo(
    query="""MATCH (e) RETURN e.name, LABELS(e) as label ORDER BY label, e.name""",
    description='Returns each node in the graph, specifing node label.',
    max_run_time_ms=0.2,
    expected_result=[['Amsterdam','country'],
                     ['Andora','country'],
                     ['Canada','country'],
                     ['China','country'],
                     ['Germany','country'],
                     ['Greece','country'],
                     ['Italy','country'],
                     ['Japan','country'],
                     ['Kazakhstan','country'],
                     ['Prague','country'],
                     ['Russia','country'],
                     ['Thailand','country'],
                     ['USA','country'],
                     ['Ailon Velger','person'],
                     ['Alon Fital','person'],
                     ['Boaz Arad','person'],
                     ['Gal Derriere','person'],
                     ['Jane Chernomorin','person'],
                     ['Lucy Yanfital','person'],
                     ['Mor Yesharim','person'],
                     ['Noam Nativ','person'],
                     ['Omri Traub','person'],
                     ['Ori Laslo','person'],
                     ['Roi Lipman','person'],
                     ['Shelly Laslo Rooz','person'],
                     ['Tal Doron','person'],
                     ['Valerie Abigail Arad','person']]
)

relation_type_counts = QueryInfo(
    query="""MATCH ()-[e]->() RETURN TYPE(e) as relation_type, COUNT(e) as num_relations ORDER BY relation_type, num_relations""",
    description='Returns each relation type in the graph and its count.',
    max_run_time_ms=0.4,
    expected_result=[['friend', '13.000000'],
                     ['visited', '35.000000']]
)

subset_of_people = QueryInfo(
    query="""MATCH (p:person) RETURN p.name ORDER BY p.name SKIP 3 LIMIT 5""",
    description='Get a subset of people.',
    max_run_time_ms=0.2,
    expected_result=[["Gal Derriere"],
                    ["Jane Chernomorin"],
                    ["Lucy Yanfital"],
                    ["Mor Yesharim"],
                    ["Noam Nativ"]]
)

my_friends_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person) 
             RETURN f.name""",
    description='My friends?',
    max_run_time_ms=0.25,
    expected_result=[['Tal Doron'],
                     ['Omri Traub'],
                     ['Boaz Arad'],
                     ['Ori Laslo'],
                     ['Ailon Velger'],
                     ['Alon Fital']]
)

friends_of_friends_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->(fof:person) 
             RETURN fof.name""",
    description='Friends of friends?',
    max_run_time_ms=0.3,
    expected_result=[['Valerie Abigail Arad'],
                     ['Shelly Laslo Rooz'],
                     ['Noam Nativ'],
                     ['Jane Chernomorin'],
                     ['Mor Yesharim'],
                     ['Gal Derriere'],
                     ['Lucy Yanfital']]
)

friends_of_friends_single_and_over_30_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->(fof:person {status:"single"})
             WHERE fof.age > 30
             RETURN fof""",
    description='Friends of friends who are single and over 30?',
    max_run_time_ms=0.35,
    expected_result=[['Noam Nativ', '34.000000', 'male', 'single']]
)

friends_of_friends_visited_amsterdam_and_single_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->
                (fof:person {status:"single"})-[:visited]->(:country {name:"Amsterdam"})
             RETURN fof.name""",
    description='Friends of friends who visited Amsterdam and are single?',
    max_run_time_ms=0.35,
    expected_result=[['Noam Nativ'],
                     ['Gal Derriere']]
)

friends_visited_same_places_as_me_query = QueryInfo(
    query="""MATCH (:person {name:"Roi Lipman"})-[:visited]->(c:country)<-[:visited]-(f:person)<-
                [:friend]-(:person {name:"Roi Lipman"}) 
             RETURN f.name, c""",
    description='Friends who have been to places I have visited?',
    max_run_time_ms=0.45,
    expected_result=[['Tal Doron', 'Japan'],
                     ['Alon Fital', 'Prague'],
                     ['Tal Doron', 'USA'],
                     ['Omri Traub', 'USA'],
                     ['Boaz Arad', 'USA'],
                     ['Ori Laslo', 'USA'],
                     ['Alon Fital', 'USA']]
)

friends_older_than_me_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
             WHERE f.age > ME.age
             RETURN f.name, f.age""",
    description='Friends who are older than me?',
    max_run_time_ms=0.25,
    expected_result=[['Omri Traub', '33.000000']]
)

friends_age_difference_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
             RETURN f.name, abs(ME.age - f.age) AS age_diff
             ORDER BY age_diff desc""",
    description='Age difference between me and each of my friends.',
    max_run_time_ms=0.25,
    expected_result=[['Boaz Arad', '1.000000'],
                     ['Omri Traub', '1.000000'],
                     ['Ailon Velger', '0.000000'],
                     ['Tal Doron', '0.000000'],
                     ['Ori Laslo', '0.000000'],
                     ['Alon Fital', '0.000000']]
)

how_many_countries_each_friend_visited_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(friend:person)-[:visited]->(c:country)
             RETURN friend.name, count(c.name) AS countriesVisited
             ORDER BY countriesVisited DESC
             LIMIT 10""",
    description='Count for each friend how many countires he or she been to?',
    max_run_time_ms=0.3,
    expected_result=[['Alon Fital', '3.000000'],
                     ['Omri Traub', '3.000000'],
                     ['Tal Doron', '3.000000'],
                     ['Ori Laslo', '3.000000'],
                     ['Boaz Arad', '2.000000']]
)

happy_birthday_query = QueryInfo(
    query = """MATCH (:person {name:"Roi Lipman"})-[:friend]->(f:person)
               SET f.age = f.age + 1""",
    description='Update friends age.',
    max_run_time_ms=0.2,
    expected_result=[]
)

friends_age_statistics_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
             RETURN ME.name, count(f.name), sum(f.age), avg(f.age), min(f.age), max(f.age)""",
    description='Friends age statistics.',
    max_run_time_ms=0.25,
    expected_result=[['Roi Lipman', '6.000000', '198.000000', '33.000000', '32.000000', '34.000000']]
)

visit_purpose_of_each_country_i_visited_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[v:visited]->(c:country) 
             RETURN c.name, v.purpose""",
    description='For each country i have been to, what was the visit purpose?',
    max_run_time_ms=0.25,
    expected_result=[['Japan', 'pleasure'],
                     ['Prague', 'both'],
                     ['USA', 'business']]
)

who_was_on_business_trip_query = QueryInfo(
    query="""MATCH (p:person)-[v:visited {purpose:"business"}]->(c:country)
             RETURN p.name, v.purpose, toUpper(c.name)""",
    description='Find out who went on a business trip?',
    max_run_time_ms=0.3,
    expected_result=[['Ori Laslo', 'business', 'CHINA'],
                     ['Ori Laslo', 'business', 'USA'],
                     ['Roi Lipman', 'business', 'USA'],
                     ['Mor Yesharim', 'business', 'GERMANY'],
                     ['Tal Doron', 'business', 'JAPAN'],
                     ['Gal Derriere', 'business', 'AMSTERDAM']]
)

number_of_vacations_per_person_query = QueryInfo(
    query="""MATCH (p:person)-[v:visited {purpose:"pleasure"}]->(c:country)
             RETURN p.name, count(v.purpose) AS vacations
             ORDER BY vacations DESC
             LIMIT 6""",
    description='Count number of vacations per person?',
    max_run_time_ms=0.5,
    expected_result=[['Noam Nativ', '3.000000'],
                     ['Shelly Laslo Rooz', '3.000000'],
                     ['Omri Traub', '3.000000'],
                     ['Lucy Yanfital', '2.000000'],
                     ['Jane Chernomorin', '2.000000'],
                     ['Mor Yesharim', '2.000000'],
                     ['Valerie Abigail Arad', '2.000000']]
)

all_reachable_friends_query = QueryInfo(
    query="""MATCH (a:person {name:'Roi Lipman'})-[:friend*]->(b:person)
             RETURN b.name
             ORDER BY b.name""",
    description='Find all reachable friends',
    max_run_time_ms=0.3,
    expected_result=[['Ailon Velger'],
                     ['Alon Fital'],
                     ['Boaz Arad'],
                     ['Gal Derriere'],
                     ['Jane Chernomorin'],
                     ['Lucy Yanfital'],
                     ['Mor Yesharim'],
                     ['Noam Nativ'],
                     ['Omri Traub'],
                     ['Ori Laslo'],
                     ['Shelly Laslo Rooz'],
                     ['Tal Doron'],
                     ['Valerie Abigail Arad']]
)

all_reachable_countries_query = QueryInfo(
    query="""MATCH (a:person {name:'Roi Lipman'})-[*]->(c:country)
             RETURN c.name, count(c.name) AS NumPathsToCountry
             ORDER BY NumPathsToCountry DESC""",
    description='Find all reachable countries',
    max_run_time_ms=0.7,
    expected_result=[['USA', '9.000000'],
                     ['Amsterdam', '5.000000'],
                     ['Greece', '4.000000'],
                     ['Prague', '3.000000'],
                     ['Germany', '2.000000'],
                     ['Andora', '2.000000'],
                     ['Japan', '2.000000'],
                     ['Canada', '2.000000'],
                     ['China', '2.000000'],
                     ['Kazakhstan', '1.000000'],
                     ['Thailand', '1.000000'],
                     ['Italy', '1.000000'],
                     ['Russia', '1.000000']]
)

reachable_countries_or_people_query = QueryInfo(
    query="""MATCH (a:person {name:'Roi Lipman'})-[:friend|:visited]->(e)
             RETURN e.name
             ORDER BY e.name""",
    description='Every person or country one hop away from source node',
    max_run_time_ms=0.5,
    expected_result=[["Boaz Arad"],
                     ["Ori Laslo"],
                     ["Ailon Velger"],
                     ["Alon Fital"],
                     ["Tal Doron"],
                     ["Omri Traub"],
                     ["USA"],
                     ["Prague"],
                     ["Japan"]]
)

all_reachable_countries_or_people_query = QueryInfo(
    query="""MATCH (a:person {name:'Roi Lipman'})-[:friend|:visited*]->(e)
             RETURN e.name, count(e.name) AS NumPathsToEntity
             ORDER BY NumPathsToEntity DESC""",
    description='Every reachable person or country from source node',
    max_run_time_ms=0.5,
    expected_result=[['USA', '9.000000'],
                     ['Amsterdam', '5.000000'],
                     ['Greece', '4.000000'],
                     ['Prague', '3.000000'],
                     ['Germany', '2.000000'],
                     ['Japan', '2.000000'],
                     ['Andora', '2.000000'],
                     ['Canada', '2.000000'],
                     ['China', '2.000000'],
                     ['Ailon Velger', '1.000000'],
                     ['Alon Fital', '1.000000'],
                     ['Gal Derriere', '1.000000'],
                     ['Jane Chernomorin', '1.000000'],
                     ['Omri Traub', '1.000000'],
                     ['Boaz Arad', '1.000000'],
                     ['Noam Nativ', '1.000000'],
                     ['Shelly Laslo Rooz', '1.000000'],
                     ['Russia', '1.000000'],
                     ['Valerie Abigail Arad', '1.000000'],
                     ['Mor Yesharim', '1.000000'],
                     ['Italy', '1.000000'],
                     ['Tal Doron', '1.000000'],
                     ['Thailand', '1.000000'],
                     ['Kazakhstan', '1.000000'],
                     ['Lucy Yanfital', '1.000000'],
                     ['Ori Laslo', '1.000000']]
)

all_reachable_entities_query = QueryInfo(
    query="""MATCH (a:person {name:'Roi Lipman'})-[*]->(e)
             RETURN e.name, count(e.name) AS NumPathsToEntity
             ORDER BY NumPathsToEntity DESC""",
    description='Find all reachable entities',
    max_run_time_ms=0.5,
    expected_result=[['USA', '9.000000'],
                     ['Amsterdam', '5.000000'],
                     ['Greece', '4.000000'],
                     ['Prague', '3.000000'],
                     ['Germany', '2.000000'],
                     ['Japan', '2.000000'],
                     ['Andora', '2.000000'],
                     ['Canada', '2.000000'],
                     ['China', '2.000000'],
                     ['Ailon Velger', '1.000000'],
                     ['Alon Fital', '1.000000'],
                     ['Gal Derriere', '1.000000'],
                     ['Jane Chernomorin', '1.000000'],
                     ['Omri Traub', '1.000000'],
                     ['Boaz Arad', '1.000000'],
                     ['Noam Nativ', '1.000000'],
                     ['Shelly Laslo Rooz', '1.000000'],
                     ['Russia', '1.000000'],
                     ['Valerie Abigail Arad', '1.000000'],
                     ['Mor Yesharim', '1.000000'],
                     ['Italy', '1.000000'],
                     ['Tal Doron', '1.000000'],
                     ['Thailand', '1.000000'],
                     ['Kazakhstan', '1.000000'],
                     ['Lucy Yanfital', '1.000000'],
                     ['Ori Laslo', '1.000000']]
)

delete_friendships_query = QueryInfo(
    query="""MATCH (ME:person {name:'Roi Lipman'})-[e:friend]->() DELETE e""",
    description='Delete frienships',
    max_run_time_ms=0.5,
    expected_result=[]
)

delete_person_query = QueryInfo(
    query="""MATCH (ME:person {name:'Roi Lipman'}) DELETE ME""",
    description='Delete myself from the graph',
    max_run_time_ms=0.5,
    expected_result=[]
)

post_delete_label_query = QueryInfo(
    query="""MATCH (p:person) RETURN p.name""",
    description='Retrieve all nodes with person label',
    max_run_time_ms=0.5,
    expected_result=[['Boaz Arad'],
                     ['Valerie Abigail Arad'],
                     ['Ori Laslo'],
                     ['Shelly Laslo Rooz'],
                     ['Ailon Velger'],
                     ['Noam Nativ'],
                     ['Jane Chernomorin'],
                     ['Alon Fital'],
                     ['Mor Yesharim'],
                     ['Gal Derriere'],
                     ['Lucy Yanfital'],
                     ['Tal Doron'],
                     ['Omri Traub']]
)

queries_info = [
    graph_entities,
    relation_type_counts,
    subset_of_people,
    my_friends_query,
    friends_of_friends_query,
    friends_of_friends_single_and_over_30_query,
    friends_of_friends_visited_amsterdam_and_single_query,
    friends_visited_same_places_as_me_query,
    friends_older_than_me_query,
    friends_age_difference_query,
    how_many_countries_each_friend_visited_query,
    happy_birthday_query,
    friends_age_statistics_query,
    visit_purpose_of_each_country_i_visited_query,
    who_was_on_business_trip_query,
    number_of_vacations_per_person_query,
    all_reachable_friends_query,
    all_reachable_countries_query,
    reachable_countries_or_people_query,
    all_reachable_countries_or_people_query,
    all_reachable_entities_query,
    delete_friendships_query,
    delete_person_query,
    post_delete_label_query
]
