#!/usr/bin/env python3

import os
import sys
import argparse
import subprocess
from time import time
from RLTest import Env
from redis import ResponseError
from redis.commands.graph import Graph

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "../../deps/readies"))
import paella


def make_connection():
    env = Env(decodeResponses=True, module="../../src/redisgraph.so", logDir="logs")
    redis_con = env.getConnection()
    return Graph(redis_con, "G")


def issue_queries(graph, timeout):
    working_dir = os.path.abspath(os.path.dirname(__file__))
    env = dict(os.environ, PYTHONPATH=os.pathsep.join([os.environ.get('PYTHONPATH', ''), working_dir + "/generator"]))
    cmd = ["grammarinator-generate CustomCypherGenerator.CustomCypherGenerator --sys-path generator/ --jobs 1 -r oC_Query --stdout -d 30"]

    start = time()
    while time() - start < timeout:
        # Capture generated queries
        try:
            query = subprocess.check_output(cmd, env=env, shell=True, encoding="utf8")
        except subprocess.CalledProcessError as e:
            # Subprocess failed, emit the thrown exception and retry.
            print(str(e))
            continue

        # Log query to console
        print(query)
        try:
            graph.query(query, timeout=1000)
        except ResponseError as e:
            print("Encountered exception: %s" % str(e))
        except ConnectionError as e:
            # The previous query crashed the server, exit with exception.
            raise(e)


def runner():
    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--timeout", type=int, default=30, help="timeout value in seconds")
    args = parser.parse_args()
    timeout = args.timeout

    graph = make_connection()

    issue_queries(graph, timeout)


# Invoke
runner()
