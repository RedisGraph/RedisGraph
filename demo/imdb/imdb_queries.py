import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

number_of_actors_query = QueryInfo(
    query="""MATCH (n:actor) RETURN count(n) as actors_count""",
    description='How many actors are in the graph?',
    max_run_time_ms=0.2,
    expected_result=[[1317]]
)

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
             RETURN a.name, a.age, m.title, m.year, m.votes, m.rating, m.genre ORDER BY ID(a)""",
    description='Which actors who are over 50 played in blockbuster movies?',
    max_run_time_ms=4.0,
    expected_result=[['Bill Irwin', 69, 'Interstellar', 2014, 961763, '8.6', 'Adventure'],
                     ['Vincent Price', 108, 'Vincent', 1982, 18284, '8.4', 'Short'],
                     ['Ellen Burstyn', 87, 'Interstellar', 2014, 961763, '8.6', 'Adventure'],
                     ['Paul Reiser', 62, 'Whiplash', 2014, 420586, '8.5', 'Drama'],
                     ['Francis X. McCarthy', 77, 'Interstellar', 2014, 961763, '8.6', 'Adventure'],
                     ['John Lithgow', 74, 'Interstellar', 2014, 961763, '8.6', 'Adventure'],
                     ['J.K. Simmons', 64, 'Whiplash', 2014, 420586, '8.5', 'Drama',],
                     ['Chris Mulkey', 71, 'Whiplash', 2014, 420586, '8.5', 'Drama'],
                     ['Rachael Harris', 51, 'Lucifer', 2015, 58703, '8.3', 'Crime'],
                     ['Matthew McConaughey', 50, 'Interstellar', 2014, 961763, '8.6', 'Adventure'],
                     ['D.B. Woodside', 50, 'Lucifer', 2015, 58703, '8.3', 'Crime']]

)


actors_played_in_bad_drama_or_comedy_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie)
             WHERE (m.genre = "Drama" OR m.genre = "Comedy")
             AND m.rating < 5.5 AND m.votes > 50000
             RETURN a.name, m.title, m.year, m.votes, m.rating, m.genre
             ORDER BY m.rating""",
    description='Which actors played in bad drama or comedy?',
    max_run_time_ms=4,
    expected_result=[['Rita Ora', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Dakota Johnson', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Marcia Gay Harden', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Jamie Dornan', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Eloise Mumford', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Max Martini', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Luke Grimes', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Jennifer Ehle', 'Fifty Shades of Grey', 2015, 224710, '4.1', 'Drama'],
                     ['Victor Rasuk', 'Fifty Shades of Grey', 2015,224710, '4.1', 'Drama'],
                     ['Nancy Lenehan', 'Sex Tape', 2014, 86018, '5.1', 'Comedy'],
                     ['Rob Lowe', 'Sex Tape', 2014, 86018, '5.1', 'Comedy'],
                     ['Cameron Diaz', 'Sex Tape', 2014, 86018, '5.1', 'Comedy'],
                     ['Rob Corddry', 'Sex Tape', 2014, 86018, '5.1', 'Comedy'],
                     ['Jason Segel', 'Sex Tape', 2014, 86018, '5.1', 'Comedy'],
                     ['Ellie Kemper', 'Sex Tape', 2014, 86018, '5.1', 'Comedy']]
)


young_actors_played_with_cameron_diaz_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < 35
             RETURN a.name, a.age, m.title ORDER BY ID(a)""",
    description='Which young actors played along side Cameron Diaz?',
    max_run_time_ms=5,
    expected_result=[['Nicolette Pierini', 16, 'Annie'],
                     ['Kate Upton', 27, 'The Other Woman']]
)


actors_played_with_cameron_diaz_and_younger_than_her_query = QueryInfo(
    query="""MATCH (Cameron:actor {name:"Cameron Diaz"})-[:act]->(m:movie)<-[:act]-(a:actor)
             WHERE a.age < Cameron.age
             RETURN a.name, a.age, m.title order by a.name""",
    description='Which actors played along side Cameron Diaz and are younger then her?',
    max_run_time_ms=7,
    expected_result=[['Jason Segel', 39, 'Sex Tape'],
                     ['Ellie Kemper', 39, 'Sex Tape'],
                     ['Nicolette Pierini', 16, 'Annie'],
                     ['Rose Byrne', 40, 'Annie'],
                     ['Kate Upton', 27, 'The Other Woman'],
                     ['Nicki Minaj', 37, 'The Other Woman'],
                     ['Taylor Kinney', 38, 'The Other Woman']]
)


sum_and_average_age_of_straight_outta_compton_cast_query = QueryInfo(
    query="""MATCH (a:actor)-[:act]->(m:movie{title:"Straight Outta Compton"})
             RETURN m.title, SUM(a.age), AVG(a.age)""",
    description='What is the sum and average age of the Straight Outta Compton cast?',
    max_run_time_ms=4,
    expected_result=[['Straight Outta Compton', '131', '32.75']]
)


how_many_movies_cameron_diaz_played_query = QueryInfo(
    query="""MATCH (Cameron:actor{name:"Cameron Diaz"})-[:act]->(m:movie)
             RETURN Cameron.name, COUNT(m.title)""",
    description='In how many movies did Cameron Diaz played?',
    max_run_time_ms=1.2,
    expected_result=[['Cameron Diaz', 3]]
)


find_ten_oldest_actors_query = QueryInfo(
    query="""MATCH (a:actor)
             RETURN DISTINCT a.name, a.age
             ORDER BY a.age DESC
             LIMIT 10""",
    description='10 Oldest actors?',
    max_run_time_ms=4.5,
    expected_result=[['Vincent Price', 108],
                     ['George Kennedy', 94],
                     ['Cloris Leachman', 93],
                     ['John Cullum', 89],
                     ['Lois Smith', 89],
                     ['Robert Duvall', 88],
                     ['Olympia Dukakis', 88],
                     ['Ellen Burstyn', 87],
                     ['Michael Caine', 86],
                     ['Judi Dench', 85]]
)

actors_over_85_index_scan = QueryInfo(
    query="""MATCH (a:actor)
             WHERE a.age > 85
             RETURN a.name, a.age
             ORDER BY a.age, a.name""",
    description='Actors over 85 on indexed property?',
    max_run_time_ms=1.5,
    expected_result=[['Michael Caine', 86],
                     ['Ellen Burstyn', 87],
                     ['Robert Duvall', 88],
                     ['Olympia Dukakis', 88],
                     ['Lois Smith', 89],
                     ['John Cullum', 89],
                     ['Cloris Leachman', 93],
                     ['George Kennedy', 94],
                     ['Vincent Price', 108]]
)

eighties_movies_index_scan = QueryInfo(
    query="""MATCH (m:movie)
             WHERE 1980 <= m.year
             AND m.year < 1990
             RETURN m.title, m.year
             ORDER BY m.year""",
    description='Multiple filters on indexed property?',
    max_run_time_ms=1.5,
    expected_result=[['The Evil Dead', 1981],
                     ['Vincent', 1982]]
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
    number_of_actors_query,
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
