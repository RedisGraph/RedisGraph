import os
import sys
import unittest
import redis
from .base import FlowTestsBase

GRAPH_ID = "multiexec-graph"
redis_con = None
dis_redis = None

# Fully connected 3 nodes graph,
CREATE_QUERY = """CREATE (al:person {name:'Al'}), (betty:person {name:'Betty'}), (michael:person {name:'Michael'}),
                    (al)-[:knows]->(betty), (al)-[:knows]->(michael),
                    (betty)-[:knows]->(al), (betty)-[:knows]->(michael),
                    (michael)-[:knows]->(al), (michael)-[:knows]->(betty)"""
CREATE_QUERY = CREATE_QUERY.replace("\n", "")

# Count outgoing connections from Al.
MATCH_QUERY = """MATCH (al:person {name:'Al'})-[]->(b:person) RETURN al, count(b)"""

# Disconnect edge connecting Al to Betty.
DEL_QUERY = """MATCH (al:person {name:'Al'})-[e:knows]->(b:person {name:'Betty'}) DELETE e"""

# Change Al name from Al to Steve.
UPDATE_QUERY = "MATCH (al:person {name:'Al'}) SET al.name = 'Steve'"

def get_redis():
    global dis_redis
    conn = redis.Redis()
    try:
        conn.ping()
        # Assuming RedisGraph is loaded.
    except redis.exceptions.ConnectionError:
        # Bring up our own redis-server instance.
        from .redis_base import DisposableRedis
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn

class MultiExecFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "Multi Exec"
        global redis_con
        redis_con = get_redis()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

    def test_graph_entities(self):
        # Delete previous graph if exists.
        redis_con.execute_command("DEL", GRAPH_ID)

        # Start a multi exec transaction.
        redis_con.execute_command("MULTI")

        # Create graph.
        redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, CREATE_QUERY)

        # Count outgoing connections from Al, expecting 2 edges.
        # (Al)-[e]->() count (e)
        redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, MATCH_QUERY)

        # Disconnect edge connecting Al to Betty.
        # (Al)-[e]->(Betty) delete (e)
        redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, DEL_QUERY)

        # Count outgoing connections from Al, expecting 1 edges.
        # (Al)-[e]->() count (e)
        redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, MATCH_QUERY)

        # Change Al name from Al to Steve.
        # (Al) set Al.name = Steve
        redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, UPDATE_QUERY)

        # Count outgoing connections from Al, expecting 0 edges.
        # (Al)-[e]->() count (e)
        redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, MATCH_QUERY)

        # Commit transaction.
        results = redis_con.execute_command("EXEC")

        # [
        #   [
        #       ['al.name', 'count(b)'],
        #       ['Al', '2']
        #   ],
        #       ['Query internal execution time: 0.143000 milliseconds']
        # ]

        # Get MATCH results from queued execution.
        two_edges = results[1]
        two_edges = two_edges[0][1][1]
        two_edges = int(float(two_edges))
        assert(two_edges == 2)

        one_edge = results[3]
        one_edge = one_edge[0][1][1]
        one_edge = int(float(one_edge))
        assert(one_edge == 1)

        no_edges = results[5]
        no_edges = no_edges[0]
        assert(len(no_edges) == 1)

if __name__ == '__main__':
    unittest.main()
