import sys

sys.path.append('../../')
from Demo import QueryInfo


actors_played_with_nicolas_cage_query = QueryInfo(
    query="""MATCH (n:actor{name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
             RETURN a.name, m.title""",
    description='Which actors played along side Nicolas Cage?',
    max_run_time_ms=3,
    expected_result=''
)


find_three_actors_played_with_nicolas_cage_query = QueryInfo(
    query="""MATCH (nicolas:actor {name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
             RETURN a.name, m.title
             LIMIT 3""",
    description='Get 3 actors who have played along side Nicolas Cage?',
    max_run_time_ms=3,
    expected_result=''
)


actors_played_in_movie_straight_outta_compton_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie {title:"Straight Outta Compton"})
             RETURN a.name""",
    description='Which actors played in the movie Straight Outta Compton?',
    max_run_time_ms=3,
    expected_result=''
)


actors_over_50_that_played_in_blockbusters_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie)
             WHERE a.age >= 50 AND m.votes > 10000 AND m.rating > 8.2
             RETURN a, m""",
    description='Which actors who are over 50 played in blockbuster movies?',
    max_run_time_ms=3,
    expected_result=''
)


actors_played_in_bad_drama_or_comedy_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie)
             WHERE (m.gener = Drama OR m.gener = Comedy)
             AND m.rating < 5.5 AND m.votes > 50000
             RETURN a.name, m
             ORDER BY m.rating""",
    description='Which actors played in bad drama or comedy?',
    max_run_time_ms=3,
    expected_result=''
)


young_actors_played_with_cameron_diaz_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < 35
             RETURN a, m.title""",
    description='Which young actors played along side Cameron Diaz?',
    max_run_time_ms=3,
    expected_result=''
)


actors_played_with_cameron_diaz_and_younger_than_her_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < Cameron.age
             RETURN a, m.title""",
    description='Which actors played along side Cameron Diaz and are younger then her?',
    max_run_time_ms=3,
    expected_result=''
)


sum_and_average_age_of_straight_outta_compton_cast_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie{title:"Straight Outta Compton"})
             RETURN m.title, SUM(a.age), AVG(a.age)""",
    description='What is the sum and average age of the Straight Outta Compton cast?',
    max_run_time_ms=3,
    expected_result=''
)


how_many_movies_cameron_diaz_played_query = QueryInfo(
    query="""MATCH (Cameron:actor{name:"Cameron Diaz"})-[:act]->(m:movie)
             RETURN Cameron.name, COUNT(m.title)""",
    description='In how many movies did Cameron Diaz played?',
    max_run_time_ms=3,
    expected_result=''
)


find_ten_oldest_actors_query = QueryInfo(
    query="""MATCH (a:actor)
             RETURN DISTINCT a.name, a.age
             ORDER BY a.age DESC
             LIMIT 10""",
    description='10 Oldest actors?',
    max_run_time_ms=3,
    expected_result=''
)


queries_info = [
    actors_played_with_nicolas_cage_query,
    find_three_actors_played_with_nicolas_cage_query,
    actors_played_in_movie_straight_outta_compton_query,
    actors_over_50_that_played_in_blockbusters_query,
    actors_played_in_bad_drama_or_comedy_query,
    young_actors_played_with_cameron_diaz_query,
    actors_played_with_cameron_diaz_and_younger_than_her_query,
    sum_and_average_age_of_straight_outta_compton_cast_query,
    how_many_movies_cameron_diaz_played_query,
    find_ten_oldest_actors_query
]

