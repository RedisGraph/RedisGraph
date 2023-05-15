from common import *

def expect_error(graph, env, query, expected_err_msg):
    try:
        graph.query(query)
        assert(False)
    except redis.exceptions.ResponseError as e:
        env.assertIn(expected_err_msg, str(e))
