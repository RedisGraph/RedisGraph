from itertools import permutations
from RLTest import Env
from enum import Enum
import random

class Connection(Enum):
    Connected = 1
    Disconnected = 2

keys = {
    '{x}x_35339586-b605-4d28-a6e4-51d86b5ea398': b'\x07\x81\x82\xb6\xa9\x86g\xadh\t\x05\x02x\x00\x02\x1e\x02\x00\x02\x01\x02\x00\x02\x03\x02\x01\x02\x01\x02\n\x02\x00\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x01\x02\x01\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x02\x02\x02\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x03\x02\x03\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x04\x02\x04\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x05\x02\x05\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x06\x02\x06\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x07\x02\x07\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x08\x02\x08\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\t\x02\t\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\n\x00\t\x00\xc3\x1e\xbd1\xca\xf9\x95H',
    '{x}x_7880d0f5-62c9-4e4b-b33e-acd1ddbe3f68': b'\x07\x81\x82\xb6\xa9\x86g\xadh\t\x05\x02x\x00\x02\x1e\x02\x00\x02\x01\x02\x00\x02\x03\x02\x01\x02\x01\x02\n\x02\n\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0b\x02\x0b\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0c\x02\x0c\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\r\x02\r\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0e\x02\x0e\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x0f\x02\x0f\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x10\x02\x10\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x11\x02\x11\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x12\x02\x12\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x13\x02\x13\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x14\x00\t\x00\xb9\x83\x9e\xeb\xe4\xf2\xac\xde',
    'x': b'\x07\x81\x82\xb6\xa9\x85\xd6\xadh\t\x05\x02x\x00\x02\x1e\x02\x00\x02\x01\x02\x00\x02\x03\x02\x05\x02\x01\x02\n\x02\x02\x02\x00\x02\x03\x02\x00\x02\x04\x02\x00\x02\x05\x02\x01\x02\x14\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x15\x02\x15\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x16\x02\x16\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x17\x02\x17\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x18\x02\x18\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x19\x02\x19\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1a\x02\x1a\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1b\x02\x1b\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1c\x02\x1c\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1d\x02\x1d\x02\x01\x02\x00\x02\x01\x02\x00\x02`\x00\x02\x1e\x02\x01\x05\x02v\x00\x02\x01\x02\x00\x05\x02N\x00\x02\x00\x02\x00\x00\t\x00\xb1\x90\xb5?\xc3\x0b\xef\xe6'
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
        self.connection_state = Connection.Connected

        # skip test if we're running under Valgrind
        if self.env.envRunner.debugger is not None:
            self.env.skip() # valgrind is not working correctly with replication

    # check that the expected key count exists in both master and slave
    def _check(self, keys_master, keys_slave):
        if keys_master is not None:
            keys = self.master.keys('*')
            self.env.assertEqual(len(keys), keys_master)
        if keys_slave is not None:
            # the WAIT command forces master slave sync to complete
            self.master.execute_command("WAIT", "1", "0")
            keys = self.slave.keys('*')
            self.env.assertEqual(len(keys), keys_slave)
            if keys_master is not None:
                keys = self.master.keys('*')
                self.env.assertEqual(len(keys), keys_master)

    # restore the key data and validate the # of keys
    def _step(self, key, keys_master):
        self.master.restore(key, '0', keys[key])
        self._check(keys_master, None)

    # validate that the imported data exists in both master and slave
    def _test_data(self):
        expected = [[i] for i in range(1, 31)]
        q = "MATCH (n:N) RETURN n.v"
        result = self.master.execute_command("GRAPH.RO_QUERY", "x", q)
        self.env.assertEqual(result[1], expected)
        result = self.slave.execute_command("GRAPH.RO_QUERY", "x", q)
        self.env.assertEqual(result[1], expected)
    
    def _connect_replication(self):
        if self.connection_state == Connection.Disconnected:
            self.slave.slaveof(self.master_host, self.master_port)
            self.connection_state = Connection.Connected

    def _disconnect_replication(self):
        if self.connection_state == Connection.Connected:
            self.slave.slaveof()
            self.connection_state = Connection.Disconnected

    def _connection_permutation(self, state, i):
        if state[i] == 1:
            self._connect_replication()   
        elif state[i] == 0:
            self._disconnect_replication()
    
    def _permutation(self, r, d):
        for i in range(r ** d):
            res = []
            for j in range(d):
                res.append((i >> j) % 2)
            yield res

    def _choose_random(self, iter, k):
        is_random = True
        if is_random:
            return random.choices(list(iter), k=k)
        return iter

    def test_replication_permutations(self):
        for scenario in self._choose_random(permutations(keys.keys()), 2):
            print(f"scenario: {scenario}")
            for connection_permutation in self._choose_random(self._permutation(2, 5), 3):
                print(f"connection_permutation: {connection_permutation}")
                self.master.flushall()
                self._check(0, 0)

                self._connection_permutation(connection_permutation, 0)

                aux = self.master.execute_command("GRAPH.DEBUG", "AUX", "START")
                self.env.assertEqual(aux, 1)

                self._connection_permutation(connection_permutation, 1)

                self._step(scenario[0], 1)

                self._connection_permutation(connection_permutation, 2)

                self._step(scenario[1], 2)

                self._connection_permutation(connection_permutation, 3)

                self._step(scenario[2], 3)

                self._connection_permutation(connection_permutation, 4)

                aux = self.master.execute_command("GRAPH.DEBUG", "AUX", "END")
                self.env.assertEqual(aux, 0)

                self._connect_replication()

                self._check(1, 1)
                self._test_data()
