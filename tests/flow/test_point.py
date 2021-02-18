import os
import sys
from RLTest import Env
from redisgraph import Graph

GRAPH_ID = "point"
redis_graph = None

class testPath():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def setUp(self):
        self.env.flush()

    def assert_distance(self, a, b, expected_distance):
        # A is 18.07 km away from B
        q = """WITH point({latitude:%f, longitude:%f}) AS a,
        point({latitude:%f, longitude:%f}) AS b
        RETURN distance(a, b)""" % (a['lat'], a['lon'], b['lat'], b['lon'])

        distance = redis_graph.query(q).result_set[0][0]
        error_rate = 0.1 * expected_distance
        self.env.assertAlmostEqual(distance, expected_distance, error_rate)

    def test_point_distance(self):
        # 0 m apart
        a = {'lat': 32.070794860, 'lon': 34.820751118}
        expected_distance = 0
        self.assert_distance(a, a, expected_distance)

        # 160 m apart
        a = {'lat': 32.070794860, 'lon': 34.820751118}
        b = {'lat': 32.070109656, 'lon': 34.822351298}
        expected_distance = 160
        self.assert_distance(a, b, expected_distance)

        # 11352 km apart
        a = {'lat': 32.070794860, 'lon': 34.820751118}
        b = {'lat': 30.621734079, 'lon': -96.33775507}
        expected_distance = 11352120
        self.assert_distance(a, b, expected_distance)

    def test_point_values(self):
        try:
            # latitude > 90
            q = "RETURN point({latitude:90.1, longitude:20}) AS p"
            res = redis_graph.query(q)
            assert(False)
        except Exception as e:
            # Expecting an error.
            self.env.assertIn('latitude should be within', str(e))

        try:
            # latitude < -90
            q = "RETURN point({latitude:-90.1, longitude:20}) AS p"
            res = redis_graph.query(q)
            assert(False)
        except Exception as e:
            # Expecting an error.
            self.env.assertIn('latitude should be within', str(e))

        try:
            # longitude > 180
            q = "RETURN point({latitude:10, longitude:180.1}) AS p"
            res = redis_graph.query(q)
            assert(False)
        except Exception as e:
            # Expecting an error.
            self.env.assertIn('longitude should be within', str(e))

        try:
            # longitude < -180
            q = "RETURN point({latitude:10, longitude:-180.1}) AS p"
            res = redis_graph.query(q)
            assert(False)
        except Exception as e:
            # Expecting an error.
            self.env.assertIn('longitude should be within', str(e))

    def test_point_index_lookup(self):
        home = {'lat': 32.070794860, 'lon': 34.820751118}
        univ = {'lat': 30.621734079, 'lon': -96.33775507}
        kiosk = {'lat': 32.07011414663042, 'lon': 34.82235394761603}
        austin = {'lat': 30.274919961709788, 'lon': -97.7403239617543}
        miradouro = {'lat': 37.854010999507736, 'lon': -25.775820972037057}

        # create index over location
        q = "create index on :N(loc)"
        redis_graph.query(q)

        # create 2 points: 'home' and 'univ'
        q = """create (:N {name:'home', loc:point({ latitude:%f, longitude:%f })})""" % (home['lat'], home['lon'])
        redis_graph.query(q)
        q = """create (:N {name:'univ', loc:point({ latitude:%f, longitude:%f })})""" % (univ['lat'], univ['lon'])
        redis_graph.query(q)

        # validate that the entities were created and can be returned properly
        q = """match (n:N) RETURN n.name, n.loc ORDER BY n.name"""
        actual_result = redis_graph.query(q)
        self.env.assertEquals(actual_result.result_set[0][0], "home")
        self.env.assertAlmostEqual(actual_result.result_set[0][1]["latitude"], 32.070794860, 1e-5)
        self.env.assertAlmostEqual(actual_result.result_set[0][1]["longitude"], 34.820751118, 1e-5)

        self.env.assertEquals(actual_result.result_set[1][0], "univ")
        self.env.assertAlmostEqual(actual_result.result_set[1][1]["latitude"], 30.621734079, 1e-5)
        self.env.assertAlmostEqual(actual_result.result_set[1][1]["longitude"], -96.33775507, 1e-5)

        idx_q = """MATCH (n:N) WHERE distance(n.loc, point({latitude:%f, longitude:%f})) < %d RETURN n.name"""
        none_idx_q = """MATCH (n) WHERE distance(n.loc, point({latitude:%f, longitude:%f})) < %d RETURN n.name"""

        # make sure index is being utilized
        plan = redis_graph.execution_plan(idx_q % (32.12, 43.32, 100))
        self.env.assertIn('Index Scan', plan)

        # near kiosk (200m)
        distance = 200
        idx_res = redis_graph.query(idx_q % (kiosk['lat'], kiosk['lon'], distance)).result_set
        none_idx_res = redis_graph.query(none_idx_q % (kiosk['lat'], kiosk['lon'], distance)).result_set

        # expecting just a single result: 'home'
        self.env.assertEquals(len(idx_res), 1)
        self.env.assertEquals(idx_res, none_idx_res)
        self.env.assertEquals(idx_res[0][0], 'home')

        # near "Miradouro do Cerrado das Freiras" (100km)
        # expecting no results
        distance = 100000
        idx_res = redis_graph.query(idx_q % (miradouro['lat'], miradouro['lon'], distance)).result_set
        none_idx_res = redis_graph.query(none_idx_q % (miradouro['lat'], miradouro['lon'], distance)).result_set
        self.env.assertEquals(idx_res, none_idx_res)
        self.env.assertEquals(len(idx_res), 0)

        # near Austin Texas (200km)
        # expecting just a single result: 'univ'
        distance = 200000
        idx_res = redis_graph.query(idx_q % (austin['lat'], austin['lon'], distance)).result_set
        none_idx_res = redis_graph.query(none_idx_q % (austin['lat'], austin['lon'], distance)).result_set
        self.env.assertEquals(idx_res, none_idx_res)
        self.env.assertEquals(len(idx_res), 1)
        self.env.assertEquals(idx_res[0][0], 'univ')

