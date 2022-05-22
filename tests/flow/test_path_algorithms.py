from random import randint
from common import *
from functools import cmp_to_key

class testAllShortestPaths():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        self.graph = Graph(redis_con, "all_shortest_paths")
        self.cyclic_graph = Graph(redis_con, "all_shortest_paths_cyclic")
        self.populate_graph()

    def populate_graph(self):
        # direction
        # weight not numeric

        NODES = 50
        EDGES = 1000

        self.graph.query("CREATE INDEX ON :L(v)")
        self.graph.query(f"UNWIND range(1, {NODES}) AS x CREATE (:L{{v: x}})")
        self.graph.query(f"UNWIND range(1, {EDGES}) AS i WITH ToInteger(rand() * 100) AS x, ToInteger(rand() * 100) AS y MATCH (a:L{{v: x}}), (b:L{{v: y}}) CREATE (a)-[:E {{weight: ToInteger(rand()*5) + 1, cost: ToInteger(rand()*10) + 3}}]->(b)")

        self.n = 0
        self.m = 0
        self.sp_paths = []
        self.incoming_sp_paths = []
        self.ss_paths = []
        for i in range(1, NODES):
            for j in range(1, NODES):
                if i == j:
                    continue

                query = f"""
                MATCH (n:L {{v: {i}}}), (m:L {{v: {j}}})
                MATCH p=(n)-[:E*1..3]->(m)
                RETURN p, reduce(weight = 0, r in relationships(p) | weight + r.weight) AS weight, reduce(cost = 0, r in relationships(p) | cost + r.cost) AS cost, length(p) as pathLen"""

                result = self.graph.query(query)
                l = len(result.result_set)
                if l > 10:
                    self.n = i
                    self.m = j
                    self.sp_paths = result.result_set

                    query = f"""
                    MATCH (n:L {{v: {i}}})
                    MATCH p=(n)-[:E*1..3]->(m)
                    RETURN p, reduce(weight = 0, r in relationships(p) | weight + r.weight) AS weight, reduce(cost = 0, r in relationships(p) | cost + r.cost) AS cost, length(p) as pathLen"""

                    result = self.graph.query(query)
                    self.ss_paths = result.result_set

                    query = f"""
                    MATCH (n:L {{v: {i}}}), (m:L {{v: {j}}})
                    MATCH p=(m)<-[:E*1..3]-(n)
                    RETURN p, reduce(weight = 0, r in relationships(p) | weight + r.weight) AS weight, reduce(cost = 0, r in relationships(p) | cost + r.cost) AS cost, length(p) as pathLen"""

                    result = self.graph.query(query)
                    self.incoming_sp_paths = result.result_set
                    break

        def compare_cost(p1,p2):
            return p1[2] - p2[2]

        def compare_full(p1,p2):
            if p1[1] == p2[1]:
                if p1[2] == p2[2]:
                    return p1[3] - p2[3]
                return p1[2] - p2[2]
            return p1[1] - p2[1]
        
        self.sp_paths.sort(key=cmp_to_key(compare_cost))
        self.max_cost = self.sp_paths[7][2]
        self.sp_paths = [p for p in self.sp_paths if p[2] <= self.max_cost]
        self.sp_paths.sort(key=cmp_to_key(compare_full))
        # for p in self.sp_paths:
        #     print(p)
        
        self.incoming_sp_paths = [p for p in self.incoming_sp_paths if p[2] <= self.max_cost]
        self.incoming_sp_paths.sort(key=cmp_to_key(compare_full))
        
        self.ss_paths = [p for p in self.ss_paths if p[2] <= self.max_cost]
        self.ss_paths.sort(key=cmp_to_key(compare_full))
        # for p in self.ss_paths:
        #     print(p)

    def test00_validate_algorithms(self):
        queries = [
                """CALL algo.SPpaths({})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: n})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({targetNode: n})"""
            ]
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("sourceNode and targetNode are required", str(e))

        queries = [
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: 1, targetNode: 1})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: 1, targetNode: n})""",
                """MATCH (n:L {v: 1}) CALL algo.SPpaths({sourceNode: n, targetNode: 1})"""
            ]
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("sourceNode and targetNode must be of type Node", str(e))
        
        queries = [
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relTypes: 1})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relTypes: [1]})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relTypes: ['a', 1]})"""
            ]
        for query in queries:
            try:
                self.graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("relTypes must be array of strings", str(e))

        queries = [
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relDirection: 1})""",
                """MATCH (n:L {v: 1}), (m:L {v: 5}) CALL algo.SPpaths({sourceNode: n, targetNode: m, relDirection: 'a'})"""
            ]
        for query in queries:
            try:
                self.graph.query(query)
                print(query)
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


    def test01_sp_single_path(self):
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 1}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path)"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(result.result_set[0], self.sp_paths[0])

        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: m, targetNode: n, relTypes: ['E'], relDirection: 'incoming', maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 1}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path)"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(result.result_set[0], self.incoming_sp_paths[0])

    def test02_sp_all_minimal_paths(self):    
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 0}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen
        ORDER BY pathWeight, pathCost, pathLen"""
        
        result = self.graph.query(query)
        
        all_minimal = [p for p in self.sp_paths if p[1] == self.sp_paths[0][1]]
        self.env.assertEquals(len(result.result_set), len(all_minimal))
        for i in range(0, len(all_minimal)):
            self.env.assertEquals(result.result_set[i], self.sp_paths[i])

        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: m, targetNode: n, relTypes: ['E'], relDirection: 'incoming', maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 0}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen
        ORDER BY pathWeight, pathCost, pathLen"""
        
        result = self.graph.query(query)
        
        all_minimal = [p for p in self.incoming_sp_paths if p[1] == self.incoming_sp_paths[0][1]]
        self.env.assertEquals(len(result.result_set), len(all_minimal))
        for i in range(0, len(all_minimal)):
            self.env.assertEquals(result.result_set[i], self.incoming_sp_paths[i])
    
    def test03_sp_k_minimal_paths(self):    
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 5}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen
        ORDER BY pathWeight, pathCost, pathLen"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 5)
        for i in range(0, 5):
            self.env.assertEquals(result.result_set[i], self.sp_paths[i])
        
        query = f"""
        MATCH (n:L {{v: {self.n}}}), (m:L {{v: {self.m}}})
        CALL algo.SPpaths({{sourceNode: m, targetNode: n, relTypes: ['E'], relDirection: 'incoming', maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 5}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen
        ORDER BY pathWeight, pathCost, pathLen"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 5)
        for i in range(0, 5):
            self.env.assertEquals(result.result_set[i], self.incoming_sp_paths[i])

    def test04_ss_single_path(self):
        query = f"""
        MATCH (n:L {{v: {self.n}}})
        CALL algo.SSpaths({{sourceNode: n, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 1}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path)"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(result.result_set[0], self.ss_paths[0])

    def test05_ss_all_minimal_paths(self):    
        query = f"""
        MATCH (n:L {{v: {self.n}}})
        CALL algo.SSpaths({{sourceNode: n, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 0}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen
        ORDER BY pathWeight, pathCost, pathLen"""
        
        result = self.graph.query(query)

        all_minimal = [p for p in self.ss_paths if p[1] == self.ss_paths[0][1]]
        self.env.assertEquals(len(result.result_set), len(all_minimal))
        for i in range(0, len(all_minimal)):
            self.env.assertEquals(result.result_set[i], self.ss_paths[i])
    
    def test06_ss_k_minimal_paths(self):    
        query = f"""
        MATCH (n:L {{v: {self.n}}})
        CALL algo.SSpaths({{sourceNode: n, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: {self.max_cost}, pathCount: 5}}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost, length(path) as pathLen
        ORDER BY pathWeight, pathCost, pathLen"""
        
        result = self.graph.query(query)

        self.env.assertEquals(len(result.result_set), 5)
        for i in range(0, 5):
            self.env.assertEquals(result.result_set[i], self.ss_paths[i])
