import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

graph_entities = QueryInfo(
    query="""MATCH (e) RETURN e.name, LABELS(e) as label ORDER BY label, e.name""",
    description='Returns each node in the graph, specifing node label.',
    max_run_time_ms=0.2,
    expected_result=[['Netherlands','country'],
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
    expected_result=[['friend', 13],
                     ['visited', 43]]
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
    max_run_time_ms=0.2,
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
    max_run_time_ms=0.2,
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
             RETURN fof.name, fof.age, fof.gender, fof.status""",
    description='Friends of friends who are single and over 30?',
    max_run_time_ms=0.25,
    expected_result=[['Noam Nativ', 34, 'male', 'single']]
)

friends_of_friends_visited_netherlands_and_single_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->
             (fof:person {status:"single"})-[:visited]->(:country {name:"Netherlands"})
             RETURN fof.name""",
    description='Friends of friends who visited Netherlands and are single?',
    max_run_time_ms=0.3,
    expected_result=[['Noam Nativ'],
                     ['Gal Derriere']]
)

friends_visited_same_places_as_me_query = QueryInfo(
    query="""MATCH (:person {name:"Roi Lipman"})-[:visited]->(c:country)<-[:visited]-(f:person)<-
             [:friend]-(:person {name:"Roi Lipman"}) 
             RETURN f.name, c ORDER BY f.name, c.name""",
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
    expected_result=[['Omri Traub', 33]]
)

friends_age_difference_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
             RETURN f.name, abs(ME.age - f.age) AS age_diff
             ORDER BY age_diff desc""",
    description='Age difference between me and each of my friends.',
    max_run_time_ms=0.35,
    expected_result=[['Boaz Arad', 1],
                     ['Omri Traub', 1],
                     ['Ailon Velger', 0],
                     ['Tal Doron', 0],
                     ['Ori Laslo', 0],
                     ['Alon Fital', 0]]
)

friends_who_are_older_than_average = QueryInfo(
    query="""MATCH (p:person)
             WITH avg(p.age) AS average_age 
             MATCH(:person)-[:friend]->(f:person) 
             WHERE f.age > average_age 
             RETURN f.name, f.age, round(f.age - average_age) AS age_diff 
             ORDER BY age_diff, f.name DESC
             LIMIT 4""",
    description='Friends who are older then the average age.',
    max_run_time_ms=0.35,
    expected_result=[['Noam Nativ', 34, '3'],
                     ['Omri Traub', 33, '2'],
                     ['Tal Doron', 32, '1'],
                     ['Ori Laslo', 32, '1']]
)

how_many_countries_each_friend_visited_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(friend:person)-[:visited]->(c:country)
             RETURN friend.name, count(c.name) AS countriesVisited
             ORDER BY countriesVisited DESC
             LIMIT 10""",
    description='Count for each friend how many countires he or she been to?',
    max_run_time_ms=0.3,
    expected_result=[['Alon Fital', 3],
                     ['Omri Traub', 3],
                     ['Tal Doron', 3],
                     ['Ori Laslo', 3],
                     ['Boaz Arad', 2]]
)

happy_birthday_query = QueryInfo(
    query = """MATCH (:person {name:"Roi Lipman"})-[:friend]->(f:person)
               SET f.age = f.age + 1
               RETURN f.name, f.age order by f.name, f.age""",
    description='Update friends age.',
    max_run_time_ms=0.25,
    expected_result=[['Ailon Velger', 33],
                     ['Alon Fital',   33],
                     ['Boaz Arad',    32],
                     ['Omri Traub',   34],
                     ['Ori Laslo',    33],
                     ['Tal Doron',    33]]
)

friends_age_statistics_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
             RETURN ME.name, count(f.name), sum(f.age), avg(f.age), min(f.age), max(f.age)""",
    description='Friends age statistics.',
    max_run_time_ms=0.2,
    expected_result=[['Roi Lipman', 6, '198', '33', 32, 34]]
)

visit_purpose_of_each_country_i_visited_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[v:visited]->(c:country) 
             RETURN c.name, v.purpose""",
    description='For each country i have been to, what was the visit purpose?',
    max_run_time_ms=0.2,
    expected_result=[['Japan', 'pleasure'],
                     ['Prague', 'pleasure'],
                     ['Prague', 'business'],
                     ['USA', 'business']]
)

who_was_on_business_trip_query = QueryInfo(
    query="""MATCH (p:person)-[v:visited {purpose:"business"}]->(c:country)
             RETURN p.name, v.purpose, toUpper(c.name) ORDER BY p.name, c.name""",
    description='Find out who went on a business trip?',
    max_run_time_ms=0.3,
    expected_result=[['Boaz Arad', 'business','NETHERLANDS'],
                     ['Boaz Arad', 'business','USA'],
                     ['Ori Laslo', 'business', 'CHINA'],
                     ['Ori Laslo', 'business', 'USA'],
                     ['Jane Chernomorin', 'business', 'USA'],
                     ['Alon Fital', 'business', 'USA'],
                     ['Alon Fital', 'business', 'PRAGUE'],
                     ['Mor Yesharim', 'business', 'GERMANY'],
                     ['Gal Derriere', 'business', 'NETHERLANDS'],
                     ['Lucy Yanfital', 'business', 'USA'],
                     ['Roi Lipman', 'business', 'USA'],
                     ['Roi Lipman', 'business', 'PRAGUE'],
                     ['Tal Doron', 'business', 'USA'],
                     ['Tal Doron', 'business', 'JAPAN']]
)

number_of_vacations_per_person_query = QueryInfo(
    query="""MATCH (p:person)-[v:visited {purpose:"pleasure"}]->(c:country)
             RETURN p.name, count(v.purpose) AS vacations
             ORDER BY COUNT(v.purpose), p.name DESC
             LIMIT 6""",
    description='Count number of vacations per person?',
    max_run_time_ms=0.5,
    expected_result=[['Shelly Laslo Rooz', 3],
                     ['Omri Traub', 3],
                     ['Noam Nativ', 3],
                     ['Lucy Yanfital', 3],
                     ['Jane Chernomorin', 3],
                     ['Alon Fital', 3]]
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
             ORDER BY NumPathsToCountry, c.name DESC""",
    description='Find all reachable countries',
    max_run_time_ms=0.6,
    expected_result=[['USA', 14],
                     ['Netherlands', 6],
                     ['Prague', 5],
                     ['Greece', 4],
                     ['Japan', 2],
                     ['Germany', 2],
                     ['China', 2],
                     ['Canada', 2],
                     ['Andora', 2],
                     ['Thailand', 1],
                     ['Russia', 1],
                     ['Kazakhstan', 1],
                     ['Italy', 1]]
)

reachable_countries_or_people_query = QueryInfo(
    query="""MATCH (s:person {name:'Roi Lipman'})-[e:friend|:visited]->(t)
             RETURN s.name,TYPE(e),t.name
             ORDER BY t.name""",
    description='Every person or country one hop away from source node',
    max_run_time_ms=0.2,
    expected_result=[["Roi Lipman", "friend", "Ailon Velger"],
                     ["Roi Lipman", "friend", "Alon Fital"],
                     ["Roi Lipman", "friend", "Boaz Arad"],
                     ["Roi Lipman", "visited", "Japan"],
                     ["Roi Lipman", "friend", "Omri Traub"],
                     ["Roi Lipman", "friend", "Ori Laslo"],
                     ["Roi Lipman", "visited", "Prague"],
                     ["Roi Lipman", "visited", "Prague"],
                     ["Roi Lipman", "friend", "Tal Doron"],
                     ["Roi Lipman", "visited", "USA"]]
)

all_reachable_countries_or_people_query = QueryInfo(
    query="""MATCH (a:person {name:'Roi Lipman'})-[:friend|:visited*]->(e) RETURN e.name, count(e.name) AS NumPathsToEntity ORDER BY NumPathsToEntity, e.name DESC""",
    description='Every reachable person or country from source node',
    max_run_time_ms=0.4,
    expected_result=[['USA', 14],
                     ['Netherlands', 6],
                     ['Prague', 5],
                     ['Greece', 4],
                     ['Japan', 2],
                     ['Germany', 2],
                     ['China', 2],
                     ['Canada', 2],
                     ['Andora', 2],
                     ['Valerie Abigail Arad', 1],
                     ['Thailand', 1],
                     ['Tal Doron', 1],
                     ['Shelly Laslo Rooz', 1],
                     ['Russia', 1],
                     ['Ori Laslo', 1],
                     ['Omri Traub', 1],
                     ['Noam Nativ', 1],
                     ['Mor Yesharim', 1],
                     ['Lucy Yanfital', 1],
                     ['Kazakhstan', 1],
                     ['Jane Chernomorin', 1],
                     ['Italy', 1],
                     ['Gal Derriere', 1],
                     ['Boaz Arad', 1],
                     ['Alon Fital', 1],
                     ['Ailon Velger', 1]]
)

all_reachable_entities_query = QueryInfo(
    query="""MATCH (a:person {name:'Roi Lipman'})-[*]->(e)
             RETURN e.name, count(e.name) AS NumPathsToEntity
             ORDER BY NumPathsToEntity DESC""",
    description='Find all reachable entities',
    max_run_time_ms=0.4,
    expected_result=[['USA', 14],
                     ['Netherlands', 6],
                     ['Prague', 5],
                     ['Greece', 4],
                     ['Andora', 2],
                     ['Japan', 2],
                     ['Germany', 2],
                     ['Canada', 2],
                     ['China', 2],
                     ['Ailon Velger', 1],
                     ['Alon Fital', 1],
                     ['Gal Derriere', 1],
                     ['Jane Chernomorin', 1],
                     ['Omri Traub', 1],
                     ['Boaz Arad', 1],
                     ['Noam Nativ', 1],
                     ['Shelly Laslo Rooz', 1],
                     ['Russia', 1],
                     ['Valerie Abigail Arad', 1],
                     ['Mor Yesharim', 1],
                     ['Italy', 1],
                     ['Tal Doron', 1],
                     ['Thailand', 1],
                     ['Kazakhstan', 1],
                     ['Lucy Yanfital', 1],
                     ['Ori Laslo', 1]]
)

delete_friendships_query = QueryInfo(
    query="""MATCH (ME:person {name:'Roi Lipman'})-[e:friend]->() DELETE e""",
    description='Delete frienships',
    max_run_time_ms=0.25,
    expected_result=[]
)

delete_person_query = QueryInfo(
    query="""MATCH (ME:person {name:'Roi Lipman'}) DELETE ME""",
    description='Delete myself from the graph',
    max_run_time_ms=0.2,
    expected_result=[]
)

post_delete_label_query = QueryInfo(
    query="""MATCH (p:person) RETURN p.name""",
    description='Retrieve all nodes with person label',
    max_run_time_ms=0.15,
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
    friends_of_friends_visited_netherlands_and_single_query,
    friends_visited_same_places_as_me_query,
    friends_older_than_me_query,
    friends_age_difference_query,
    friends_who_are_older_than_average,
    how_many_countries_each_friend_visited_query,
    visit_purpose_of_each_country_i_visited_query,
    who_was_on_business_trip_query,
    number_of_vacations_per_person_query,
    all_reachable_friends_query,
    all_reachable_countries_query,
    reachable_countries_or_people_query,
    all_reachable_countries_or_people_query,
    all_reachable_entities_query,
    happy_birthday_query,
    friends_age_statistics_query,
    delete_friendships_query,
    delete_person_query,
    post_delete_label_query
]
