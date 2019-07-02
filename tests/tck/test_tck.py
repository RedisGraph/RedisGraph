from RLTest import Env
from behave.__main__ import main as behave_main

# this test class is built in order to use 'behave' framework
# inside an RLTests managed environment. 
# This class is instanciated in RLTest processes during runtime and
# the call to test_tck method is intializing and executing a 'behave' 
# framework with the TCK feature tests inside the 'features' folder. 
# See /features/steps/Steps.py and utils/graphs.py to for understanding how
# 'behave' step is preformed against RLTest environment
class testTCK():
    def test_tck(self):
        env = Env()
        cmd = ["./features/", '--tags=-skip']
        if not env.verbose:
            cmd.append('--format=progress')
        res = 'pass' if behave_main(cmd) == 0 else 'fail'
        env.assertEquals(res,'pass')
 