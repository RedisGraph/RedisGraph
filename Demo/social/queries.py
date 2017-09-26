import sys

sys.path.append('../../')
from Demo import QueryInfo


my_friends_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person) 
             RETURN f.name""",
    description='My friends?',
    max_run_time_ms=3,
    expected_result=''
)


friends_of_friends_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->(fof:person) 
             RETURN fof.name""",
    description='Friends of friends?',
    max_run_time_ms=3,
    expected_result=''
)


friends_of_friends_single_and_over_30_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->(fof:person {status:single}) 
             WHERE fof.age > 30
             RETURN fof""",
    description='Friends of friends who are single and over 30?',
    max_run_time_ms=3,
    expected_result=''
)


friends_of_friends_visited_amsterdam_and_single_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(:person)-[:friend]->
                (fof:person {status:single})-[:visited]->(:country {name:Amsterdam}) 
             RETURN fof.name""",
    description='Friends of friends who visited Amsterdam and are single?',
    max_run_time_ms=3,
    expected_result=''
)


friends_visited_same_places_as_me_query = QueryInfo(
    query="""MATCH (:person {name:"Roi Lipman"})-[:visited]->(c:country)<-[:visited]-(f:person)<-
                [:friend]-(:person {name:"Roi Lipman"}) 
             RETURN f.name, c""",
    description='Friends who have been to places I have visited?',
    max_run_time_ms=3,
    expected_result=''
)


friends_older_than_me_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
             WHERE f.age > ME.age
             RETURN f.name, f.age""",
    description='Friends who are older than me?',
    max_run_time_ms=3,
    expected_result=''
)


how_many_countries_each_friend_visited_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(friend:person)-[:visited]->(c:country)
             RETURN friend.name, count(c.name) AS countriesVisited
             ORDER BY countriesVisited DESC
             LIMIT 10""",
    description='Count for each friend how many countires he or she been to?',
    max_run_time_ms=3,
    expected_result=''
)


friends_age_statistics_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[:friend]->(f:person)
             RETURN ME.name, count(f.name), sum(f.age), avg(f.age), min(f.age), max(f.age)""",
    description='Friends age statistics?',
    max_run_time_ms=3,
    expected_result=''
)


visit_purpose_of_each_country_i_visited_query = QueryInfo(
    query="""MATCH (ME:person {name:"Roi Lipman"})-[v:visited]->(c:country) 
             RETURN c.name, v.purpose""",
    description='For each country i have been to, what was the visit purpose?',
    max_run_time_ms=3,
    expected_result=''
)


who_was_on_business_trip_query = QueryInfo(
    query="""MATCH (p:person)-[v:visited {purpose:business}]->(c:country) 
             RETURN p.name, v.purpose, c.name""",
    description='Find out who went on a business trip?',
    max_run_time_ms=3,
    expected_result=''
)


number_of_vacations_per_person_query = QueryInfo(
    query="""MATCH (p:person)-[v:visited {purpose:pleasure}]->(c:country)
             RETURN p.name, count(v.purpose) AS vacations
             ORDER BY vacations DESC
             LIMIT 6""",
    description='Count number of vacations per person?',
    max_run_time_ms=3,
    expected_result=''
)


queries_info = [
    my_friends_query,
    friends_of_friends_query,
    friends_of_friends_single_and_over_30_query,
    friends_of_friends_visited_amsterdam_and_single_query,
    friends_visited_same_places_as_me_query,
    friends_older_than_me_query,
    how_many_countries_each_friend_visited_query,
    friends_age_statistics_query,
    visit_purpose_of_each_country_i_visited_query,
    who_was_on_business_trip_query,
    number_of_vacations_per_person_query
]

