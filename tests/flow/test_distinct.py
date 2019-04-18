from redis_base import RedisGraphTestBase

class ReturnDistinctFlowTest_1(RedisGraphTestBase):
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
        self.assertIn("Distinct", execution_plan)

        # Distinct should be omitted when performain aggregation.
        execution_plan = self.explain("MATCH (n) RETURN DISTINCT n.name, max(n.age)")
        self.assertNotIn("Distinct", execution_plan)

    def test_issue_395_scenario(self):
        # all
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p")
        self.assertEqual(q, [['Stevie'], ['Stevie'], ['Stevie'], ['Mike'], ['James'], ['James']])
        
        # order
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p ORDER BY p.name")
        self.assertEqual(q, [['James'], ['James'], ['Mike'], ['Stevie'], ['Stevie'], ['Stevie']])

        # limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p LIMIT 2")
        self.assertEqual(q, [['Stevie'], ['Stevie']])
        
        # order+limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p ORDER BY p.name LIMIT 2")
        self.assertEqual(q, [['James'], ['James']])
        
        # all+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p")
        self.assertEqual(q, [['Stevie'], ['Mike'], ['James']])
        
        # order+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p ORDER BY p.name")
        self.assertEqual(q, [['James'], ['Mike'], ['Stevie']])

        # limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p LIMIT 2")
        self.assertEqual(q, [['Stevie'], ['Mike']])
        
        # order+limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p ORDER BY p.name LIMIT 2")
        self.assertEqual(q, [['James'], ['Mike']])

class ReturnDistinctFlowTest_2(RedisGraphTestBase):
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

    def test_issue_395_scenario_2(self):
        # all
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p")
        self.assertEqual(q, [['Stevie'], ['Stevie'], ['Stevie'], ['Mike'], ['James'], ['James']])
        
        # order
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p ORDER BY p.name")
        self.assertEqual(q, [['James'], ['James'], ['Mike'], ['Stevie'], ['Stevie'], ['Stevie']])

        # limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p LIMIT 2")
        self.assertEqual(q, [['Stevie'], ['Stevie']])
        
        # order+limit
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN p ORDER BY p.name DESC LIMIT 2")
        self.assertEqual(q, [['Stevie'], ['Stevie']])
        
        # all+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p")
        self.assertEqual(q, [['Stevie'], ['Mike'], ['James']])
        
        # order+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p ORDER BY p.name DESC")
        self.assertEqual(q, [['Stevie'], ['Mike'], ['James']])

        # limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p LIMIT 2")
        self.assertEqual(q, [['Stevie'], ['Mike']])
        
        # order+limit+distinct
        q = self.query("MATCH (p:PARENT)-[:HAS]->(:CHILD) RETURN DISTINCT p ORDER BY p.name DESC LIMIT 2")
        self.assertEqual(q, [['Stevie'], ['Mike']])

if __name__ == '__main__':
    unittest.main()
