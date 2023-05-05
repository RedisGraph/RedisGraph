import sys
import os
from functools import wraps

from RLTest import Env, Defaults

import redis
from redis import ResponseError
from redis.commands.graph import Graph
from redis.commands.graph.node import Node
from redis.commands.graph.edge import Edge
from redis.commands.graph.path import Path
from redis.commands.graph import query_result
from redis.commands.graph.execution_plan import ExecutionPlan

from base import FlowTestsBase

if sys.version_info > (3, 0):
    Defaults.decode_responses = True

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "../../deps/readies"))
import paella

SANITIZER = os.getenv('SANITIZER', '')
VALGRIND = os.getenv('VALGRIND', '0') == '1'
CODE_COVERAGE = os.getenv('CODE_COVERAGE', '0') == '1'

OSNICK = paella.Platform().osnick
OS = paella.Platform().os
ARCH = paella.Platform().arch


def skip(always=False, on_cluster=False, on_macos=False):
    def decorate(f):
        @wraps(f)
        def wrapper(x, *args, **kwargs):
            if isinstance(x, Env):
                env = x
                x = None
            else:
                env = x.env
            if always:
                env.skip()
            if on_cluster and env.isCluster():
                env.skip()
            if on_macos and OS == 'macos':
                env.skip()
            if x is None:
                return f(env, *args, **kwargs)
            else:
                return f(x, *args, **kwargs)
        return wrapper
    return decorate
