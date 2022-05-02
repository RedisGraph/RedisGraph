from common import *

class testAllShortestPaths():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        self.graph = Graph(redis_con, "all_shortest_paths")
        self.cyclic_graph = Graph(redis_con, "all_shortest_paths_cyclic")
        self.populate_graph()
        # self.populate_cyclic_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        self.v1 = Node(label="L", properties={"v": 1})
        self.v2 = Node(label="L", properties={"v": 2})
        self.v3 = Node(label="L", properties={"v": 3})
        self.v4 = Node(label="L", properties={"v": 4})
        self.v5 = Node(label="L", properties={"v": 5})

        self.graph.add_node(self.v1)
        self.graph.add_node(self.v2)
        self.graph.add_node(self.v3)
        self.graph.add_node(self.v4)
        self.graph.add_node(self.v5)

        e = Edge(self.v1, "E", self.v2, properties={"weight": 1, "cost": 1})
        self.graph.add_edge(e)

        e = Edge(self.v2, "E", self.v3, properties={"weight": 2, "cost": 2})
        self.graph.add_edge(e)

        e = Edge(self.v3, "E", self.v4, properties={"weight": 2, "cost": 2})
        self.graph.add_edge(e)

        e = Edge(self.v2, "E", self.v4, properties={"weight": 1, "cost": 1})
        self.graph.add_edge(e)

        e = Edge(self.v2, "E", self.v5, properties={"weight": 1, "cost": 2})
        self.graph.add_edge(e)

        e = Edge(self.v4, "E", self.v5, properties={"weight": 2, "cost": 2})
        self.graph.add_edge(e)

        self.graph.flush()

    def populate_cyclic_graph(self):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)
        # (v3)-[:E2]->(v1)
        # (v4)-[:E2]->(v1)

        self.cyclic_v1 = Node(label="L", properties={"v": 1})
        self.cyclic_v2 = Node(label="L", properties={"v": 2})
        self.cyclic_v3 = Node(label="L", properties={"v": 3})
        self.cyclic_v4 = Node(label="L", properties={"v": 4})
        self.cyclic_v5 = Node(label="L", properties={"v": 5})

        self.cyclic_graph.add_node(self.cyclic_v1)
        self.cyclic_graph.add_node(self.cyclic_v2)
        self.cyclic_graph.add_node(self.cyclic_v3)
        self.cyclic_graph.add_node(self.cyclic_v4)
        self.cyclic_graph.add_node(self.cyclic_v5)

        e = Edge(self.cyclic_v1, "E", self.cyclic_v2, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v2, "E", self.cyclic_v3, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v3, "E", self.cyclic_v4, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v1, "E", self.cyclic_v5, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v5, "E2", self.cyclic_v4, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v2, "E2", self.cyclic_v4, properties={"weight": 2})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v3, "E2", self.cyclic_v1, properties={"weight": 2})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v4, "E2", self.cyclic_v1, properties={"weight": 2})
        self.cyclic_graph.add_edge(e)

        self.cyclic_graph.flush()

    def test01_spmw_single_path(self):
        query = """
        MATCH (n:L {v: 1}), (m:L {v: 5})
        CALL algo.SPMWpaths({sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: 4, pathCount: 1}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost
        LIMIT 3"""
        
        result = self.graph.query(query)
        for p in result.result_set:
            if len(p) > 0:
                print(p[0])

    def test02_spmw_all_minimal_paths(self):    
        query = """
        MATCH (n:L {v: 1}), (m:L {v: 5})
        CALL algo.SPMWpaths({sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: 4, pathCount: 0}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost
        LIMIT 3"""
        
        result = self.graph.query(query)
        for p in result.result_set:
            if len(p) > 0:
                print(p[0])
    
    def test03_spmw_k_minimal_paths(self):    
        query = """
        MATCH (n:L {v: 1}), (m:L {v: 5})
        CALL algo.SPMWpaths({sourceNode: n, targetNode: m, relTypes: ['E'], maxLen: 3, weightProp: 'weight', costProp: 'cost', maxCost: 4, pathCount: 2}) YIELD path, pathWeight, pathCost
        RETURN path, pathWeight, pathCost
        LIMIT 3"""
        
        result = self.graph.query(query)
        for p in result.result_set:
            if len(p) > 0:
                print(p[0])
