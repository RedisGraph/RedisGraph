class testTCK():
    def test_tck(self):
        from RLTest import Env 
        env = Env()
        from behave.__main__ import main as behave_main
        cmd = ["./features/", '--tags=-skip']
        if not env.verbose:
            cmd.append('--format=progress')
        res = 'pass' if behave_main(cmd) == 0 else 'fail'
        env.assertEquals(res,'pass')