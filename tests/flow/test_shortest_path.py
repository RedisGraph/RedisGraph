import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge, Path
from base import FlowTestsBase

GRAPH_ID = "shortest_path_test"
graph = None

class testShortestPath(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global graph

        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)

        self.populate_graph()

    def populate_graph(self):
        a = Node(label="Loc", properties={"name": "A"})
        b = Node(label="Loc", properties={"name": "B"})
        c = Node(label="Loc", properties={"name": "C"})
        d = Node(label="Loc", properties={"name": "D"})
        e = Node(label="Loc", properties={"name": "E"})
        f = Node(label="Loc", properties={"name": "F"})

        ab = Edge(a, "ROAD", b, properties={"cost": 50})
        ac = Edge(a, "ROAD", c, properties={"cost": 60})
        ad = Edge(a, "ROAD", d, properties={"cost": 100})
        bd = Edge(b, "ROAD", d, properties={"cost": 40})
        cd = Edge(c, "ROAD", d, properties={"cost": 40})
        ce = Edge(c, "ROAD", e, properties={"cost": 80})
        de = Edge(d, "ROAD", e, properties={"cost": 30})
        df = Edge(d, "ROAD", f, properties={"cost": 80})
        ef = Edge(e, "ROAD", f, properties={"cost": 40})
        
        global graph
        graph.add_node(a)
        graph.add_node(b)
        graph.add_node(c)
        graph.add_node(d)
        graph.add_node(e)
        graph.add_node(f)

        graph.add_edge(ab)
        graph.add_edge(ac)
        graph.add_edge(ad)
        graph.add_edge(bd)
        graph.add_edge(cd)
        graph.add_edge(ce)
        graph.add_edge(de)
        graph.add_edge(df)
        graph.add_edge(ef)

        graph.flush()

    def test_shortest_path(self):
        # Find the shotest path between a and f
        # Path:
        # a->b->d->e->f
        # Costs:
        # 0, 50, 90, 120, 160

        # [(0), [0], (1), [3], (3), [6], (4), [8], (5)]
        # [0.000000, 50.000000, 90.000000, 120.000000, 160.000000]
                
        # Find shortest path between a and f
        q = """MATCH (start:Loc{name:'A'}), (end:Loc{name:'F'}) CALL algo.shortestPath(start, end, 'cost', 'ROAD', 0) YIELD path, cost RETURN path, cost"""
        result = graph.query(q)
        path = result.result_set[0][0]
        cost = result.result_set[0][1]

        # Extract expected path from graph.
        q = """MATCH p = (a {name:'A'})-[ab]->(b{name:'B'})-[bd]->(d{name:'D'})-[de]->(e{name:'E'})-[ef]->(f{name:'F'}) RETURN p"""
        result = graph.query(q)
        expected_path = result.result_set[0][0]
        expected_cost = [0, 50, 90, 120, 160]
        
        self.env.assertEquals(path, expected_path)
        self.env.assertEquals(cost, expected_cost)
