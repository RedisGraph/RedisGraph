from RLTest import Env

keys = {
    '{x}x_dba3feaa-a949-402e-a102-280f797a479f': b'\x07\x81\x82\xb6\xa9\x86g\xadh\n\x05\x02x\x00\x02\x1e\x02\x00\x02\x01\x02\x00\x02\x03\x02\x01\x02\x01\x02\n\x02\x00\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x01\x02\x01\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x02\x02\x02\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x03\x02\x03\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x04\x02\x04\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x05\x02\x05\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x06\x02\x06\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x07\x02\x07\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x08\x02\x08\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\t\x02\t\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\n\x00\t\x00\xf8/\xe9\x81{\x16\xb2\xb5',
    '{x}x_6e357ef7-b3e2-442a-8cc2-eaa51ed9fe9b': b'\x07\x81\x82\xb6\xa9\x86g\xadh\n\x05\x02x\x00\x02\x1e\x02\x00\x02\x01\x02\x00\x02\x03\x02\x01\x02\x01\x02\n\x02\n\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0b\x02\x0b\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0c\x02\x0c\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\r\x02\r\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0e\x02\x0e\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0f\x02\x0f\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x10\x02\x10\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x11\x02\x11\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x12\x02\x12\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x13\x02\x13\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x14\x00\t\x00\x82\xb2\xca[U\x1d\x8b#',
    'x': b'\x07\x81\x82\xb6\xa9\x85\xd6\xadh\n\x05\x02x\x00\x02\x1e\x02\x00\x02\x01\x02\x00\x02\x03\x02\x05\x02\x01\x02\n\x02\x02\x02\x00\x02\x03\x02\x00\x02\x04\x02\x00\x02\x05\x02\x01\x02\x14\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x15\x02\x15\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x16\x02\x16\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x17\x02\x17\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x18\x02\x18\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x19\x02\x19\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1a\x02\x1a\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1b\x02\x1b\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1c\x02\x1c\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1d\x02\x1d\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1e\x02\x01\x05\x02v\x00\x02\x01\x02\x00\x05\x02N\x00\x02\x00\x02\x00\x00\t\x00\x95z\xe9\xc8\xaa\xa9\xe1\xe5'
}

# test to see if replication works as expected when importing data
# RedisGraph should replicate all data using virtual keys mechanism
# in case we imported part of the data validate that we replicate it correctly.

class testReplicationState():
    def __init__(self):
        self.env = Env(useSlaves=True, decodeResponses=True, env='oss', moduleArgs='VKEY_MAX_ENTITY_COUNT 10')
        self.master = self.env.getConnection()
        self.slave = self.env.getSlaveConnection()
        info = self.slave.info("Replication")
        self.master_host = info["master_host"]
        self.master_port = info["master_port"]

        # skip test if we're running under Valgrind
        if self.env.envRunner.debugger is not None:
            self.env.skip() # valgrind is not working correctly with replication

    # check that the expected keys count existed in master and slave
    def _check(self, keys_master, keys_slave):
        if keys_master:
            keys = self.master.keys('*')
            self.env.assertEqual(len(keys), keys_master)
        if keys_slave:
            # wait for the sync to be finished
            self.master.execute_command("WAIT", "1", "0")
            keys = self.slave.keys('*')
            self.env.assertEqual(len(keys), keys_slave)
            if keys_master:
                keys = self.master.keys('*')
                self.env.assertEqual(len(keys), keys_master)

    # restore the key data and validate the # of keys
    def _step(self, key, keys_master, keys_slave):
        self.master.restore(key, '0', keys[key])
        self._check(keys_master, keys_slave)

    # validate that the imported data exists in master and slave
    def _test_data(self):
        expected = [[i] for i in range(1, 31)]
        result = self.master.execute_command("GRAPH.RO_QUERY", "x", "MATCH (n:N) RETURN n.v")
        self.env.assertEqual(result[1], expected)
        result = self.slave.execute_command("GRAPH.RO_QUERY", "x", "MATCH (n:N) RETURN n.v")
        self.env.assertEqual(result[1], expected)

    # test replication when master and slave always connected
    def test_replication(self):
        self.master.flushall()

        self._step('{x}x_dba3feaa-a949-402e-a102-280f797a479f', 1, 1)
        self._step('{x}x_6e357ef7-b3e2-442a-8cc2-eaa51ed9fe9b', 2, 2)
        self._step('x', 1, 1)

        self._test_data()

    # test replication when connecting the master and slave after restoring 1 key
    def test_replication_after_import_1_key(self):
        self.master.flushall()

        # disconnect master and slave
        self.slave.slaveof()

        self._step('{x}x_dba3feaa-a949-402e-a102-280f797a479f', 1, None)

        # connect master and slave
        self.slave.slaveof(self.master_host, self.master_port)

        self._check(1, 1)

        self._step('{x}x_6e357ef7-b3e2-442a-8cc2-eaa51ed9fe9b', 2, 2)
        self._step('x', 1, 1)

        self._test_data()

    # test replication when connecting the master and slave after restoring 2 keys
    def test_replication_after_import_2_key(self):
        self.master.flushall()

        # disconnect master and slave
        self.slave.slaveof()

        self._step('{x}x_dba3feaa-a949-402e-a102-280f797a479f', 1, None)
        self._step('{x}x_6e357ef7-b3e2-442a-8cc2-eaa51ed9fe9b', 2, None)

        # connect master and slave
        self.slave.slaveof(self.master_host, self.master_port)
        
        self._check(2, 2)

        self._step('x', 1, 1)

        self._test_data()

    # test replication when connecting the master and slave after restoring all keys
    def test_replication_after_import_3_key(self):
        self.master.flushall()

        # disconnect master and slave
        self.slave.slaveof()

        self._step('{x}x_dba3feaa-a949-402e-a102-280f797a479f', 1, None)
        self._step('{x}x_6e357ef7-b3e2-442a-8cc2-eaa51ed9fe9b', 2, None)
        self._step('x', 1, None)

        # connect master and slave
        self.slave.slaveof(self.master_host, self.master_port)
        
        self._check(1, 1)

        self._test_data()
