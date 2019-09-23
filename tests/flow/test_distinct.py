from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

redis_graph = None
redis_con = None

class RedisGraphTestBase(FlowTestsBase):

    def __init__(self):
        super(RedisGraphTestBase, self).__init__()
        global redis_con
        redis_con = self.env.getConnection()
        self.create_graph()

    @classmethod
    def graphId(cls):
        return cls.__name__

    @classmethod
    def createCommand(cls):
        raise NotImplementedError("Implement createCommand")

    # several commands specified as multiine string, one command per line
    @classmethod
    def createCommands(cls):
        raise NotImplementedError("Implement createCommands")

    @classmethod
    def create_graph(cls):
        global redis_con
        global redis_graph

        redis_con.execute_command("DEL", cls.graphId())  # delete previous graph if exists

        redis_graph = Graph(cls.graphId(), redis_con)

        cmd = " ".join(map(lambda x: x.strip(), cls.createCommand().split("\n")))
        if cmd != "":
            redis_graph.query(cmd)

        for cmd in cls.createCommands().split("\n"):
            cmd = cmd.strip()
            if cmd != "":
                redis_graph.query(cmd)

    def execute(self, cmd):
        global redis_con
        return redis_con.execute_command(cmd)

    def query(self, cmd):
        global redis_graph
        q = redis_graph.query(cmd)
        return q.result_set

    def explain(self, query):
        return redis_graph.execution_plan(query)

    def multi(self):
        global redis_con
        redis_con.execute_command("MULTI")

    def exec_(self):
        return redis_con.execute_command("EXEC")

class testReturnDistinctFlow1(RedisGraphTestBase):

    def __init__(self):
        super(testReturnDistinctFlow1, self).__init__()

    @classmethod
    def createCommand(cls):
        return ""

    @classmethod
    def createCommands(cls):
        return """
            CREATE (:PARENT {name: 'Stevie'})
            CREATE (:PARENT {name: 'Mike'})
            CREATE (:PARENT {name: 'James'})
            CREATE (:PARENT {name: 'Rich'})
            MATCH (p:PARENT {name: 'Stevie'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child1'})
            MATCH (p:PARENT {name: 'Stevie'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child2'})
            MATCH (p:PARENT {name: 'Stevie'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child3'})
            MATCH (p:PARENT {name: 'Mike'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child4'})
            MATCH (p:PARENT {name: 'James'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child5'})
            MATCH (p:PARENT {name: 'James'}) CREATE (p)-[:HAS]->(c:CHILD {name: 'child6'})
        """

    def test_distinct_optimization(self):
        # Make sure we do not omit distinct when performain none aggregated projection.
        execution_plan = self.explain("MATCH (n) RETURN DISTINCT n.name, n.age")
        self.env.assertIn("Distinct", execution_plan)

        # Distinct should be omitted when performain aggregation.
        execution_plan = self.explain("MATCH (n) RETURN DISTINCT n.name, max(n.age)")
        self.env.assertNotIn("Distinct", execution_plan)

    def test_issue_395_scenario(self):
        # all
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name")
        self.env.assertEqual(q, [['Stevie'], ['Stevie'], ['Stevie'], ['Mike'], ['James'], ['James']])

        # order
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name ORDER BY p.name")
        self.env.assertEqual(q, [['James'], ['James'], ['Mike'], ['Stevie'], ['Stevie'], ['Stevie']])

        # limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name LIMIT 2")
        self.env.assertEqual(q, [['Stevie'], ['Stevie']])

        # order+limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name ORDER BY p.name LIMIT 2")
        self.env.assertEqual(q, [['James'], ['James']])

        # all+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name")
        self.env.assertEqual(q, [['Stevie'], ['Mike'], ['James']])

        # order+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name ORDER BY p.name")
        self.env.assertEqual(q, [['James'], ['Mike'], ['Stevie']])

        # limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name LIMIT 2")
        self.env.assertEqual(q, [['Stevie'], ['Mike']])

        # order+limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name ORDER BY p.name LIMIT 2")
        self.env.assertEqual(q, [['James'], ['Mike']])

class testReturnDistinctFlow2(RedisGraphTestBase):

    def __init__(self):
        super(testReturnDistinctFlow2, self).__init__()

    @classmethod
    def createCommand(cls):
        return """
            CREATE
                (s:PARENT {name: 'Stevie'}),
                (m:PARENT {name: 'Mike'}),
                (j:PARENT {name: 'James'}),
                (r:PARENT {name: 'Rich'}),
                (s)-[:HAS]->(c1:CHILD {name: 'child1'}),
                (s)-[:HAS]->(c2:CHILD {name: 'child2'}),
                (s)-[:HAS]->(c3:CHILD {name: 'child3'}),
                (m)-[:HAS]->(c4:CHILD {name: 'child4'}),
                (j)-[:HAS]->(c5:CHILD {name: 'child5'}),
                (j)-[:HAS]->(c6:CHILD {name: 'child6'})
            """

    @classmethod
    def createCommands(cls):
        return ""

    def test_issue_395_scenario_2(self):
        # all
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name")
        self.env.assertEqual(q, [['Stevie'], ['Stevie'], ['Stevie'], ['Mike'], ['James'], ['James']])

        # order
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name ORDER BY p.name")
        self.env.assertEqual(q, [['James'], ['James'], ['Mike'], ['Stevie'], ['Stevie'], ['Stevie']])

        # limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name LIMIT 2")
        self.env.assertEqual(q, [['Stevie'], ['Stevie']])

        # order+limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p.name ORDER BY p.name DESC LIMIT 2")
        self.env.assertEqual(q, [['Stevie'], ['Stevie']])

        # all+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name")
        self.env.assertEqual(q, [['Stevie'], ['Mike'], ['James']])

        # order+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name ORDER BY p.name DESC")
        self.env.assertEqual(q, [['Stevie'], ['Mike'], ['James']])

        # limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name LIMIT 2")
        self.env.assertEqual(q, [['Stevie'], ['Mike']])

        # order+limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p.name ORDER BY p.name DESC LIMIT 2")
        self.env.assertEqual(q, [['Stevie'], ['Mike']])

class testCountDistinct(FlowTestsBase):
    def __init__(self):
        global redis_con
        global redis_graph
        super(testCountDistinct, self).__init__()
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    def populate_graph(self):
        a = Node()
        b = Node()
        c = Node()
        redis_graph.add_node(a)
        redis_graph.add_node(b)
        redis_graph.add_node(c)
        redis_graph.add_edge(Edge(a, "know", b))
        redis_graph.add_edge(Edge(a, "know", b))
        redis_graph.add_edge(Edge(a, "know", c))
        redis_graph.commit()

    def test_unwind_count_distinct(self):
        query = """UNWIND [1, 2, 2, "a", "a", null] as x RETURN count(distinct x)"""
        actual_result = redis_graph.query(query)
        expected_result = [[3]]
        self.env.assertEquals(actual_result.result_set, expected_result)
    
    def test_match_count_distinct(self):
        query = """MATCH (a)-[]->(x) RETURN count(distinct x)"""
        actual_result = redis_graph.query(query)
        expected_result = [[2]]
        self.env.assertEquals(actual_result.result_set, expected_result)
