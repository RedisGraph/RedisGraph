from common import *
from functools import cmp_to_key

NODES = 20    # node count
EDGES = 200   # edge count

class testAllShortestPaths():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        self.graph = Graph(redis_con, "path_algos")
        self.populate_graph()
        self.init()

    def populate_graph(self):
        self.graph.query("CREATE INDEX ON :L(v)")
        self.graph.query(f"UNWIND range(1, {NODES}) AS x CREATE (:L{{v: x}})")
        self.graph.query(f"""UNWIND range(1, {EDGES}) AS i
                             WITH ToInteger(rand() * {NODES}) AS x, ToInteger(rand() * {NODES}) AS y
                             MATCH (a:L{{v: x}}), (b:L{{v: y}})
                             CREATE (a)-[:E {{weight: ToInteger(rand()*5) + 1, cost: ToInteger(rand()*10) + 3}}]->(b)""")

    def init(self):
        self.n = 0                   # start node ID
        self.m = 0                   # end node ID
        self.sp_paths = []           # paths between (n)->(m)
        self.incoming_sp_paths = []  # paths between (m)<-(n)
        self.ss_paths = []           # all paths expand from (n)

        # look for nodes `i` and `j` with at least 10 different paths
        # between them, stop once found
        for i in range(1, NODES):
            for j in range(1, NODES):
                if i == j:
                    continue

                query = f"""
                MATCH (n:L {{v: {i}}}), (m:L {{v: {j}}})
                MATCH p=(n)-[:E*1..3]->(m)
                RETURN p,
                       reduce(weight = 0, r in relationships(p) | weight + r.weight) AS weight,
                       reduce(cost = 0, r in relationships(p) | cost + r.cost) AS cost,
                       length(p) as pathLen"""

                result = self.graph.query(query)
                l = len(result.result_set)
                if l > 10:
                    # found nodes `i` and `j` with multiple paths
                    self.n = i
                    self.m = j
                    self.sp_paths = result.result_set

                    query = f"""
                    MATCH (n:L {{v: {i}}})
                    MATCH p=(n)-[:E*1..3]->(m)
                    RETURN p,
                           reduce(weight = 0, r in relationships(p) | weight + r.weight) AS weight,
                           reduce(cost = 0, r in relationships(p) | cost + r.cost) AS cost,
                           length(p) as pathLen"""

                    result = self.graph.query(query)
                    self.ss_paths = result.result_set

                    query = f"""
                    MATCH (n:L {{v: {i}}}), (m:L {{v: {j}}})
                    MATCH p=(m)<-[:E*1..3]-(n)
                    RETURN p,
                           reduce(weight = 0, r in relationships(p) | weight + r.weight) AS weight,
                           reduce(cost = 0, r in relationships(p) | cost + r.cost) AS cost,
                           length(p) as pathLen"""

                    result = self.graph.query(query)
                    self.incoming_sp_paths = result.result_set
                    break

        # expecting `cost` to be at p[2]
        def compare_cost(p1, p2):
            return p1[2] - p2[2]

        def compare_full(p1, p2):
            # p[1] - weight
            # p[2] - cost
            # p[3] - length
            if p1[1] == p2[1]:
                if p1[2] == p2[2]:
                    return p1[3] - p2[3]
                return p1[2] - p2[2]
            return p1[1] - p2[1]
        
        # sort shortest paths by cost
        self.sp_paths.sort(key=cmp_to_key(compare_cost))
        self.max_cost = self.sp_paths[7][2]

        # filter
        self.sp_paths = [p for p in self.sp_paths if p[2] <= self.max_cost and len(p[0].nodes()) == len(set([n.id for n in p[0].nodes()]))]
        self.ss_paths = [p for p in self.ss_paths if p[2] <= self.max_cost and len(p[0].nodes()) == len(set([n.id for n in p[0].nodes()]))]
        self.incoming_sp_paths = [p for p in self.incoming_sp_paths if p[2] <= self.max_cost and len(p[0].nodes()) == len(set([n.id for n in p[0].nodes()]))]

        # sort
        self.sp_paths.sort(key=cmp_to_key(compare_full))
        self.ss_paths.sort(key=cmp_to_key(compare_full))
        self.incoming_sp_paths.sort(key=cmp_to_key(compare_full))

        # for p in self.sp_paths:
        #     print(p)
        #     print(p[0])

    def test01_SPpaths_validations(self):
        # all queries should produce a run-time errors
        queries = [
                """CALL algo.SPpaths({})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: n})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({targetNode: n})"""
            ]

        # validate we're getting an exception
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("sourceNode and targetNode are required", str(e))

        # all queries should produce a run-time errors
        queries = [
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: 1, targetNode: 1})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: 1, targetNode: n})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: n, targetNode: 1})"""
            ]

        # validate we're getting an exception
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("sourceNode and targetNode must be of type Node", str(e))
        
        # all queries should produce a run-time errors
        queries = [
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relTypes: 1})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relTypes: [1]})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relTypes: ['a', 1]})"""
            ]

        # validate we're getting an exception
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("relTypes must be array of strings", str(e))

        # all queries should produce a run-time errors
        queries = [
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relDirection: 1})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relDirection: 'a'})"""
            ]

        # validate we're getting an exception
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("relDirection values must be 'incoming', 'outgoing' or 'both'", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, maxLen: 'a'})"""

        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("maxLen must be integer", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, weightProp: 1})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("weightProp must be a string", str(e))
        
        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, costProp: 1})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("costProp must be a string", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, maxCost: '1'})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("maxCost must be numeric", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, pathCount: '1'})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("pathCount must be integer", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, pathCount: -1})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("pathCount must be greater than or equal to 0", str(e))

    def test01_SSpaths_validations(self):
        query = """CALL algo.SSpaths({})"""

        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("sourceNode is required", str(e))

        query = """MATCH (n:L {v: 1}) CALL algo.SSpaths({sourceNode: 1})"""

        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("sourceNode must be of type Node", str(e))
        
        # all queries should produce a run-time errors
        queries = [
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, relTypes: 1})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, relTypes: [1]})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, relTypes: ['a', 1]})"""
            ]

        # validate we're getting an exception
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("relTypes must be array of strings", str(e))

        # all queries should produce a run-time errors
        queries = [
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, relDirection: 1})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, relDirection: 'a'})"""
            ]

        # validate we're getting an exception
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("relDirection values must be 'incoming', 'outgoing' or 'both'", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, maxLen: 'a'})"""

        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("maxLen must be integer", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, weightProp: 1})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("weightProp must be a string", str(e))
        
        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, costProp: 1})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("costProp must be a string", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, maxCost: '1'})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("maxCost must be numeric", str(e))

        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, pathCount: '1'})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("pathCount must be integer", str(e))
        
        query = """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SSpaths({sourceNode: n, pathCount: -1})"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("pathCount must be greater than or equal to 0", str(e))

    def test02_sp_single_path(self):
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 1}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path)"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 1)

        all_minimal = [p for p in self.sp_paths if p[1] == self.sp_paths[0][1]]
        self.env.assertContains(result.result_set[0], all_minimal)

        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: m, targetNode: n, relTypes: ['E'], relDirection: 'incoming', maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 1}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path)"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 1)

        all_minimal = [p for p in self.incoming_sp_paths if p[1] == self.incoming_sp_paths[0][1]]
        self.env.assertContains(result.result_set[0], all_minimal)

    def test03_sp_all_minimal_paths(self):
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 0}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen"""
        
        result = self.graph.query(query)
        
        all_minimal = [p for p in self.sp_paths if p[1] == self.sp_paths[0][1]]
        self.env.assertEquals(len(result.result_set), len(all_minimal))
        for i in range(0, len(all_minimal)):
            self.env.assertContains(result.result_set[i], all_minimal)

        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: m, targetNode: n, relTypes: ['E'], relDirection: 'incoming', maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 0}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen"""
        
        result = self.graph.query(query)
        
        all_minimal = [p for p in self.incoming_sp_paths if p[1] == self.incoming_sp_paths[0][1]]
        self.env.assertEquals(len(result.result_set), len(all_minimal))
        for i in range(0, len(all_minimal)):
            self.env.assertContains(result.result_set[i], all_minimal)
    
    def test04_sp_k_minimal_paths(self):    
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 5}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen"""
        
        result = self.graph.query(query)

        expected_len = min(len(self.sp_paths), 5)
        self.env.assertEquals(len(result.result_set), expected_len)
        for i in range(0, expected_len):
            self.env.assertContains(result.result_set[i], self.sp_paths)
        
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: m, targetNode: n, relTypes: ['E'], relDirection: 'incoming', maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 5}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), expected_len)
        for i in range(0, 5):
            self.env.assertContains(result.result_set[i], self.incoming_sp_paths)

    def test05_ss_single_path(self):
        query = f"""
        MATCH (n:L {{v: {self.n}}})
        CALL algo.SSpaths({{sourceNode: n, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 1}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path)"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(result.result_set[0], self.ss_paths[0])

    def test06_ss_all_minimal_paths(self):    
        query = f"""
        MATCH (n:L {{v: {self.n}}})
        CALL algo.SSpaths({{sourceNode: n, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 0}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen"""
        
        result = self.graph.query(query)

        all_minimal = [p for p in self.ss_paths if p[1] == self.ss_paths[0][1]]
        self.env.assertEquals(len(result.result_set), len(all_minimal))
        for i in range(0, len(all_minimal)):
            self.env.assertContains(result.result_set[i], all_minimal)
    
    def test07_ss_k_minimal_paths(self):    
        query = f"""
        MATCH (n:L {{v: {self.n}}})
        CALL algo.SSpaths({{sourceNode: n, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 5}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 5)
        for i in range(0, 5):
            self.env.assertContains(result.result_set[i], self.ss_paths)

