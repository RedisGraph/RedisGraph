from RLTest import Env
import multiprocessing as mp
from redisgraph import Graph

con = None

def init_process_local_connection(env):
    global con
    con = env.getConnection()

class Multiproc:

    # create and init n_procs workers and their connections which will execute fn(args)
    def __init__(self, env, n_procs):

        # Forcefully setting the ctx to fork so it will be possible to pass env in initargs on macOS
        self.ctx = mp.get_context('fork')

        # creating connetions on pool init for increasing the fn calls conccurency
        # we are passing the env and not the connection so that a new connection will be created for each process
        self.pool = self.ctx.Pool(n_procs, initializer=init_process_local_connection, initargs=(env,))

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.pool.terminate()

    # get a fn to run async and it args and returns AsyncResult
    def run_async(self, fn, args=tuple()):
        return self.pool.apply_async(fn, args=args)

    # executes fns asynchronously,
    # doesn't wait for all functions to finish before returning
    def add_work_no_wait(self, fns=[], args=[tuple()]):
        assert len(fns) == len(args) and len(fns) != 0
        async_calls = [self.run_async(fn, args_i) for fn, args_i in zip(fns, args)]
        return async_calls

    # executes fns asynchronously,
    # waits for all functions to finish before collecting return values into an array
    def add_work_wait(self, fns=[], args=[tuple()]):        
        ret = []
        async_calls = self.add_work_no_wait(fns, args)
        for res in async_calls:
            try:
                r = res.get()
                ret = ret + r
            except Exception as e:
                ret.append(e)
        return ret

    # runs queries on multi proccesses
    def run_queries_multiproc_no_wait(self, queries=[tuple()], graph_ids=[tuple()], nrep=1):
        assert len(queries) == len(graph_ids)
        args = []
        for q,g in zip(queries, graph_ids):
            args.append((q,g,nrep))
        return self.add_work_no_wait([issue_queries]*len(queries), args)

# create and init n_procs workers and their connections which will execute fn(args)
def run_multiproc(env, fns=[], args=[tuple()]):
        with Multiproc(env, len(fns)) as mp:
            return mp.add_work_wait(fns, args)

def issue_queries(queries, graph_ids, nrep):
    ret = []
    global con

    for i in range(nrep):
        for q,g_id in zip(queries, graph_ids):
            g = Graph(g_id, con)
            res = g.query(q)
            res.graph.redis_con = None
            ret.append(res)
    return ret

# runs queries on multi proccesses
def run_queries_multiproc(env, queries=[tuple()], graph_ids=[tuple()], nrep=1):
    assert len(queries) == len(graph_ids)
    args = []
    for q,g in zip(queries, graph_ids):
        args.append((q,g,nrep))
    return run_multiproc(env, [issue_queries]*len(queries), args)

def issue_commands(cmds, nrep):
    ret = []
    global con

    for i in range(nrep):
        for c in cmds:
            ret.append(con.execute_command(*c))
    return ret

# runs commands on multi proccesses
def run_commands_multiproc(env, commands=[tuple()], nrep=1):
    args = []
    for c in commands:
        args.append((c,nrep))
    return run_multiproc(env, [issue_commands]*len(commands), args)
