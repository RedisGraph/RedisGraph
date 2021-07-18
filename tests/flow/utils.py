from RLTest import Env
import multiprocessing as mp

con = None

class Multiproc:

    # create and init n_procs workers and their connections which will execute fn(args)
    def __init__(self, env, n_procs):
        def init_process_local_connection():
            global con
            con = env.getConnection()
            return 1

        # on macOS the spawn start method is now the default one since python 3.8.
        # the spawn start methon fails when it stumbles upon objects that contain locks/fds
        # For this reason need to forcefully set the context to fork.
        # For more details see: https://bugs.python.org/issue33725
        # or the python docs: https://docs.python.org/3/library/multiprocessing.html#contexts-and-start-methods
        self.ctx = mp.get_context('fork')

        # crating connetion on pool init for increasing the fn calls rate 
        self.pool = self.ctx.Pool(n_procs, initializer=init_process_local_connection)
    
    def __del__(self):
        self.pool.terminate()

    def __enter__(self):
        self.pool._check_running()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.pool.terminate()
    
    # gets a tuple of funcs and it's corresponding arguments and execute them using the workers.
    # wait for all work to end and return the results
    def add_work_wait(self, fns=tuple(), args=tuple(tuple())):
        results = multiple_results = None
        assert len(fns) == len(args) and len(fns) != 0
        for fn,args_i in zip(fns, args):
            multiple_results.extend(run_async(fn, args_i))
            results.extend([res.get() for res in multiple_results])
        return results

    # get a fn to run async and it args and returns AsyncResult
    def run_async(self, fn, args=tuple()):
        return [self.pool.apply_async(fn, args=args)]

    # create and init n_procs workers and their connections which will execute fn(args)
    @staticmethod
    def run_test_multiproc(env, n_procs, fn, args=tuple()):
        def init_process_local_connection():
            global con
            con = env.getConnection()
            return 1

        # on macOS the spawn start method is now the default one since python 3.8.
        # the spawn start methon fails when it stumbles upon objects that contain locks/fds
        # For this reason need to forcefully set the context to fork.
        # For more details see: https://bugs.python.org/issue33725
        # or the python docs: https://docs.python.org/3/library/multiprocessing.html#contexts-and-start-methods
        ctx = mp.get_context('fork')
        # crating connetion on pool init for increasing the fn calls rate 
        with ctx.Pool(n_procs, initializer=init_process_local_connection) as p:
            multiple_results = [p.apply_async(fn, args=args) for i in range(n_procs)]
            results = [res.get() for res in multiple_results]
            return results