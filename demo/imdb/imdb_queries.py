import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo


actors_played_with_nicolas_cage_query = QueryInfo(
    query="""MATCH (n:actor{name:"Nicolas Cage"})-[:act]->(m:movie)<-[:act]-(a:actor)
             RETURN a.name, m.title""",
    description='Which actors played along side Nicolas Cage?',
    max_run_time_ms=4,
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
    max_run_time_ms=4,
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
    max_run_time_ms=4.0,
    expected_result=[['Bill Irwin', '68.000000', 'Interstellar', '2014.000000', '961763.000000', '8.600000', 'Adventure'],
                     ['Vincent Price', '107.000000', 'Vincent', '1982.000000', '18284.000000', '8.400000', 'Short'],
                     ['Ellen Burstyn', '86.000000', 'Interstellar', '2014.000000', '961763.000000', '8.600000', 'Adventure'],
                     ['Paul Reiser', '61.000000', 'Whiplash', '2014.000000', '420586.000000', '8.500000', 'Drama'],
                     ['Francis X. McCarthy', '76.000000', 'Interstellar', '2014.000000', '961763.000000', '8.600000', 'Adventure'],
                     ['John Lithgow', '73.000000', 'Interstellar', '2014.000000', '961763.000000', '8.600000', 'Adventure'],
                     ['J.K. Simmons', '63.000000', 'Whiplash', '2014.000000', '420586.000000', '8.500000', 'Drama',],
                     ['Chris Mulkey', '70.000000', 'Whiplash', '2014.000000', '420586.000000', '8.500000', 'Drama'],
                     ['Rachael Harris', '50.000000', 'Lucifer', '2015.000000', '58703.000000', '8.300000', 'Crime']]
)


actors_played_in_bad_drama_or_comedy_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie)
             WHERE (m.genre = "Drama" OR m.genre = "Comedy")
             AND m.rating < 5.5 AND m.votes > 50000
             RETURN a.name, m
             ORDER BY m.rating""",
    description='Which actors played in bad drama or comedy?',
    max_run_time_ms=4,
    expected_result=[['Rita Ora', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Dakota Johnson', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Marcia Gay Harden', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Jamie Dornan', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Eloise Mumford', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Max Martini', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Luke Grimes', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Jennifer Ehle', 'Fifty Shades of Grey', '2015.000000', '224710.000000', '4.100000', 'Drama'],
                     ['Victor Rasuk', 'Fifty Shades of Grey', '2015.000000','224710.000000', '4.100000', 'Drama'],
                     ['Nancy Lenehan', 'Sex Tape', '2014.000000', '86018.000000', '5.100000', 'Comedy'],
                     ['Rob Lowe', 'Sex Tape', '2014.000000', '86018.000000', '5.100000', 'Comedy'],
                     ['Cameron Diaz', 'Sex Tape', '2014.000000', '86018.000000', '5.100000', 'Comedy'],
                     ['Rob Corddry', 'Sex Tape', '2014.000000', '86018.000000', '5.100000', 'Comedy'],
                     ['Jason Segel', 'Sex Tape', '2014.000000', '86018.000000', '5.100000', 'Comedy'],
                     ['Ellie Kemper', 'Sex Tape', '2014.000000', '86018.000000', '5.100000', 'Comedy']]
)


young_actors_played_with_cameron_diaz_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < 35
             RETURN a, m.title""",
    description='Which young actors played along side Cameron Diaz?',
    max_run_time_ms=5,
    expected_result=[['Nicolette Pierini', '15.000000', 'Annie'],
                     ['Kate Upton', '26.000000', 'The Other Woman']]
)


actors_played_with_cameron_diaz_and_younger_than_her_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < Cameron.age
             RETURN a, m.title""",
    description='Which actors played along side Cameron Diaz and are younger then her?',
    max_run_time_ms=7,
    expected_result=[['Jason Segel', '38.000000', 'Sex Tape'],
                     ['Ellie Kemper', '38.000000', 'Sex Tape'],
                     ['Nicolette Pierini', '15.000000', 'Annie'],
                     ['Rose Byrne', '39.000000', 'Annie'],
                     ['Kate Upton', '26.000000', 'The Other Woman'],
                     ['Nicki Minaj', '36.000000', 'The Other Woman'],
                     ['Taylor Kinney', '37.000000', 'The Other Woman']]
)


sum_and_average_age_of_straight_outta_compton_cast_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie{title:"Straight Outta Compton"})
             RETURN m.title, SUM(a.age), AVG(a.age)""",
    description='What is the sum and average age of the Straight Outta Compton cast?',
    max_run_time_ms=4,
    expected_result=[['Straight Outta Compton', '127.000000', '31.750000']]
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
    expected_result=[['Vincent Price', '107.000000'],
                     ['George Kennedy', '93.000000'],
                     ['Cloris Leachman', '92.000000'],
                     ['John Cullum', '88.000000'],
                     ['Lois Smith', '88.000000'],
                     ['Robert Duvall', '87.000000'],
                     ['Olympia Dukakis', '87.000000'],
                     ['Ellen Burstyn', '86.000000'],
                     ['Michael Caine', '85.000000'],
                     ['Judi Dench', '84.000000']]
)

actors_over_85_index_scan = QueryInfo(
    query="""MATCH (a:actor)
             WHERE a.age > 85
             RETURN a.name, a.age
             ORDER BY a.age""",
    description='Actors over 85 on indexed property?',
    max_run_time_ms=1.5,
    expected_result=[['Ellen Burstyn', '86.000000'],
        ['Robert Duvall', '87.000000'],
        ['Olympia Dukakis', '87.000000'],
        ['Lois Smith', '88.000000'],
        ['John Cullum', '88.000000'],
        ['Cloris Leachman', '92.000000'],
        ['George Kennedy', '93.000000'],
        ['Vincent Price', '107.000000']]
)

eighties_movies_index_scan = QueryInfo(
    query="""MATCH (m:movie)
             WHERE 1980 <= m.year
             AND m.year < 1990
             RETURN m.title, m.year
             ORDER BY m.year""",
    description='Multiple filters on indexed property?',
    max_run_time_ms=1.5,
    expected_result=[['The Evil Dead', '1981.000000'],
                     ['Vincent', '1982.000000']]
)

find_titles_starting_with_american_query = QueryInfo(
    query="""MATCH (m:movie)
             WHERE LEFT(m.title, 8) = 'American'
             RETURN m.title
             ORDER BY m.title""",
    description='Movies starting with "American"?',
    max_run_time_ms=4,
    expected_result=[['American Honey'],
                     ['American Pastoral'],
                     ['American Sniper']]
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
    find_ten_oldest_actors_query,
    actors_over_85_index_scan,
    eighties_movies_index_scan,
    find_titles_starting_with_american_query
]
