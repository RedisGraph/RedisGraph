from RLTest import Env
import multiprocessing as mp

con = None

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