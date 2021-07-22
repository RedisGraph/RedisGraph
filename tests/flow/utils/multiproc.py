from RLTest import Env
import multiprocessing as mp

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
        print(args)
        return self.pool.apply_async(fn, args=args)

    # executes fns asynchronously, 
    # waits for all functions to finish before collecting return values into an array
    def add_work_wait(self, fns=[], args=[tuple()]):
        assert len(fns) == len(args) and len(fns) != 0
        ret = []
        print("BBBBBB")
        async_calls = [self.run_async(fn, args_i) for fn, args_i in zip(fns, args)]
        for res in async_calls:
            try:
                r = res.get()
                ret = ret + r
            except Exception as e:
                print("DDDDD")
                print(e)
                ret.append(e)
        return ret

def issue_queries(queries, graph_ids, nrep):
    ret = []
    global con

    for i in range(nrep):
        for q,g in zip(queries, graph_ids):
            ret.append(con.execute_command("GRAPH.QUERY", g, q))
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

def issue_fns(fns, args_list, nrep):
    ret = []
    global con
    sddfsf
    print("AAAAA")
    print(fns)
    print(args_list)

    for i in range(nrep):
        for fn,args in zip(fns,args_list):
            #print(fn, (*args))
            ret.append(fn(con, *args))
    return ret


# create and init n_procs workers and their connections which will execute fn(args)
def run_multiproc(env, fns=[], args=[tuple()]):
        with Multiproc(env, len(fns)) as mp:
            return mp.add_work_wait(fns, args)

# runs functions on multi proccesses
def run_fns_multiproc(env, fns=[tuple()], args=[tuple()], nrep=1):
    assert len(fns) == len(args) 
    _args = []
    for fn,arg in zip(fns, args):
        _args.append((fn,arg,nrep))
    print(_args)
    return run_multiproc(env, [issue_fns]*len(fns), _args)