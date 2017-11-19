import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo


actors_played_with_nicolas_cage_query = QueryInfo(
    query="""MATCH (n:actor{name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
             RETURN a.name, m.title""",
    description='Which actors played along side Nicolas Cage?',
    max_run_time_ms=3,
    expected_result=[['Cassi Thomson', 'Left Behind'],
                     ['Gary Grubbs', 'Left Behind'],
                     ['Quinton Aaron', 'Left Behind'],
                     ['Martin Klebba', 'Left Behind'],
                     ['Lea Thompson', 'Left Behind'],
                     ['Nicolas Cage', 'Left Behind'],
                     ['Chad Michael Murray', 'Left Behind'],
                     ['Jordin Sparks', 'Left Behind']]
)


find_three_actors_played_with_nicolas_cage_query = QueryInfo(
    query="""MATCH (nicolas:actor {name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
             RETURN a.name, m.title
             LIMIT 3""",
    description='Get 3 actors who have played along side Nicolas Cage?',
    max_run_time_ms=2.5,
    expected_result=[['Cassi Thomson', 'Left Behind'],
                     ['Gary Grubbs', 'Left Behind'],
                     ['Quinton Aaron', 'Left Behind'],
                     ['Martin Klebba', 'Left Behind'],
                     ['Lea Thompson', 'Left Behind'],
                     ['Nicolas Cage', 'Left Behind'],
                     ['Chad Michael Murray', 'Left Behind'],
                     ['Jordin Sparks', 'Left Behind']]
)


actors_played_in_movie_straight_outta_compton_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie {title:"Straight Outta Compton"})
             RETURN a.name""",
    description='Which actors played in the movie Straight Outta Compton?',
    max_run_time_ms=3.5,
    expected_result=[['Aldis Hodge'],
                     ['Corey Hawkins'],
                     ['Neil Brown Jr.'],
                     ['O\'Shea Jackson Jr.']]
)


actors_over_50_that_played_in_blockbusters_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie)
             WHERE a.age >= 50 AND m.votes > 10000 AND m.rating > 8.2
             RETURN a, m""",
    description='Which actors who are over 50 played in blockbuster movies?',
    max_run_time_ms=3.5,
    expected_result=[['Bill Irwin', '67.000000', 'Interstellar', '961763.000000', '2014.000000', 'Adventure', '8.600000'],
                     ['Vincent Price', '106.000000', 'Vincent', '18284.000000', '1982.000000', 'Short', '8.400000'],
                     ['Ellen Burstyn', '85.000000', 'Interstellar', '961763.000000', '2014.000000', 'Adventure', '8.600000'],
                     ['Paul Reiser', '60.000000', 'Whiplash', '420586.000000', '2014.000000', 'Drama', '8.500000'],
                     ['Francis X. McCarthy', '75.000000', 'Interstellar', '961763.000000', '2014.000000', 'Adventure', '8.600000'],
                     ['John Lithgow', '72.000000', 'Interstellar', '961763.000000', '2014.000000', 'Adventure', '8.600000'],
                     ['J.K. Simmons', '62.000000', 'Whiplash', '420586.000000', '2014.000000', 'Drama', '8.500000'],
                     ['Chris Mulkey', '69.000000', 'Whiplash', '420586.000000', '2014.000000', 'Drama', '8.500000']]
)


actors_played_in_bad_drama_or_comedy_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie)
             WHERE (m.gener = Drama OR m.gener = Comedy)
             AND m.rating < 5.5 AND m.votes > 50000
             RETURN a.name, m
             ORDER BY m.rating""",
    description='Which actors played in bad drama or comedy?',
    max_run_time_ms=4,
    expected_result=[['Eloise Mumford', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Victor Rasuk', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Max Martini', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Luke Grimes', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Rita Ora', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Marcia Gay Harden', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Dakota Johnson', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Jamie Dornan', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Jennifer Ehle', 'Fifty Shades of Grey', '224710.000000', '2015.000000', 'Drama', '4.100000'],
                     ['Rob Corddry', 'Sex Tape', '86018.000000', '2014.000000', 'Comedy', '5.100000'],
                     ['Ellie Kemper', 'Sex Tape', '86018.000000', '2014.000000', 'Comedy', '5.100000'],
                     ['Jason Segel', 'Sex Tape', '86018.000000', '2014.000000', 'Comedy', '5.100000'],
                     ['Nancy Lenehan', 'Sex Tape', '86018.000000', '2014.000000', 'Comedy', '5.100000'],
                     ['Rob Lowe', 'Sex Tape', '86018.000000', '2014.000000', 'Comedy', '5.100000'],
                     ['Cameron Diaz', 'Sex Tape', '86018.000000', '2014.000000', 'Comedy', '5.100000']]
)


young_actors_played_with_cameron_diaz_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < 35
             RETURN a, m.title""",
    description='Which young actors played along side Cameron Diaz?',
    max_run_time_ms=3.5,
    expected_result=[['Nicolette Pierini', '14.000000', 'Annie'],
                     ['Kate Upton', '25.000000', 'The Other Woman']]
)


actors_played_with_cameron_diaz_and_younger_than_her_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < Cameron.age
             RETURN a, m.title""",
    description='Which actors played along side Cameron Diaz and are younger then her?',
    max_run_time_ms=7,
    expected_result=[['Jason Segel', '37.000000', 'Sex Tape'],
                     ['Ellie Kemper', '37.000000', 'Sex Tape'],
                     ['Nicolette Pierini', '14.000000', 'Annie'],
                     ['Rose Byrne', '38.000000', 'Annie'],
                     ['Kate Upton', '25.000000', 'The Other Woman'],
                     ['Nicki Minaj', '35.000000', 'The Other Woman'],
                     ['Taylor Kinney', '36.000000', 'The Other Woman']]
)


sum_and_average_age_of_straight_outta_compton_cast_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie{title:"Straight Outta Compton"})
             RETURN m.title, SUM(a.age), AVG(a.age)""",
    description='What is the sum and average age of the Straight Outta Compton cast?',
    max_run_time_ms=4,
    expected_result=[['Straight Outta Compton', '123.000000', '30.750000']]
)


how_many_movies_cameron_diaz_played_query = QueryInfo(
    query="""MATCH (Cameron:actor{name:"Cameron Diaz"})-[:act]->(m:movie)
             RETURN Cameron.name, COUNT(m.title)""",
    description='In how many movies did Cameron Diaz played?',
    max_run_time_ms=1.2,
    expected_result=[['Cameron Diaz', '3.000000']]
)


find_ten_oldest_actors_query = QueryInfo(
    query="""MATCH (a:actor)
             RETURN DISTINCT a.name, a.age
             ORDER BY a.age DESC
             LIMIT 10""",
    description='10 Oldest actors?',
    max_run_time_ms=4.5,
    expected_result=[['Vincent Price', '106.000000'],
                     ['George Kennedy', '92.000000'],
                     ['Cloris Leachman', '91.000000'],
                     ['John Cullum', '87.000000'],
                     ['Lois Smith', '87.000000'],
                     ['Robert Duvall', '86.000000'],
                     ['Olympia Dukakis', '86.000000'],
                     ['Ellen Burstyn', '85.000000'],
                     ['Michael Caine', '84.000000'],
                     ['Judi Dench', '83.000000']]
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

