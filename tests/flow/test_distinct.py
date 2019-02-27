from redis_base import RedisGraphTestBase

class ReturnDistinctFlowTest(RedisGraphTestBase):
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

if __name__ == '__main__':
    unittest.main()
