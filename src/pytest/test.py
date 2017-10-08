from rmtest import ModuleTestCase
import unittest

class GraphTestCase(ModuleTestCase('../redisgraph.so')):

    def create_graph(self, r):
        """Creates a simple graph"""
        r.flushdb()
        r.hmset('Roi', {'age':'32', 'name': 'Roi'})
        r.hmset('Alon', {'age': '32', 'name': 'Alon'})
        r.hmset('Omri', {'age': '33', 'name': 'Omri'})
        r.hmset('Boaz', {'age': '31', 'name': 'Boaz'})
        self.assertOk(r.execute_command(
            'graph.ADDEDGE', 'social', 'Roi', 'friend', 'Alon'))
        self.assertOk(r.execute_command(
            'graph.ADDEDGE', 'social', 'Roi', 'friend', 'Omri'))
        self.assertOk(r.execute_command(
            'graph.ADDEDGE', 'social', 'Roi', 'friend', 'Boaz'))

    def tes_graph_add(self):
        """Validates adding edge to graph"""
        with self.redis() as r:
            r.flushdb()
            r.execute_command(
                'graph.ADDEDGE', 'social', 'Roi Lipman', 'friend', 'Alon Fital')

    def tes_graph_remove(self):
        """Validates removing edge from graph"""
        with self.redis() as r:
            r.flushdb()

            r.execute_command(
                'graph.ADDEDGE', 'social', 'Roi Lipman', 'friend', 'Alon Fital')

            self.assertOk(r.execute_command(
                'graph.REMOVEEDGE', 'social', 'Roi Lipman', 'friend', 'Alon Fital'))

            # Second deletion
            self.assertOk(r.execute_command(
                'graph.REMOVEEDGE', 'social', 'Roi Lipman', 'friend', 'Alon Fital'))

    def tes_graph_delete(self):
        """Validates graph deletion"""
        with self.redis() as r:
            r.flushdb()
            r.execute_command(
                'graph.ADDEDGE', 'social', 'Roi Lipman', 'friend', 'Alon Fital')

            self.assertOk(r.execute_command('graph.DELETE', 'social'))

    def test_agg_sum(self):
        """Validates sum aggregate function"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY', 'social', 'MATCH (ME:Roi)-[friend]->(f) RETURN sum(f.age)')

            age_sum = int(float(result_set[0]))
            self.assertEqual(age_sum, 96)

    def test_agg_avg(self):
        """Validates avg aggregate function"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY', 'social', 'MATCH (ME:Roi)-[friend]->(f) RETURN avg(f.age)')

            age_avg = int(float(result_set[0]))
            self.assertEqual(age_avg, 32)

    def test_agg_max(self):
        """Validates max aggregate function"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY', 'social', 'MATCH (ME:Roi)-[friend]->(f) RETURN max(f.age)')

            age_max = int(float(result_set[0]))
            self.assertEqual(age_max, 33)

    def test_agg_min(self):
        """Validates min aggregate function"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY', 'social', 'MATCH (ME:Roi)-[friend]->(f) RETURN min(f.age)')

            age_min = int(float(result_set[0]))
            self.assertEqual(age_min, 31)

    def test_agg_count(self):
        """Validates count aggregate function"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY', 'social', 'MATCH (ME:Roi)-[friend]->(f) RETURN count(f.age)')

            age_count = int(float(result_set[0]))
            self.assertEqual(age_count, 3)

    def test_limit(self):
        """Validates limited result-set"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY', 'social', 'MATCH (ME:Roi)-[friend]->(f) RETURN f.name LIMIT 2'
            )

            # additional record for run time stats
            self.assertEqual(len(result_set), 3)

    def test_order(self):
        """Validates result-set order default is ascending"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY',
                'social',
                'MATCH (ME:Roi)-[friend]->(f) RETURN f.age ORDER BY f.age')

            for i in range(len(result_set)-2):
                age = int(float(result_set[i]))
                next_age = int(float(result_set[i+1]))
                self.assertLessEqual(age, next_age)

    def test_order_asc(self):
        """Validates result-set order, default is ascending"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY',
                'social',
                'MATCH (ME:Roi)-[friend]->(f) RETURN f.age ORDER BY f.age ASC')

            for i in range(len(result_set)-2):
                age = int(float(result_set[i]))
                next_age = int(float(result_set[i+1]))
                self.assertLessEqual(age, next_age)

    def test_order_desc(self):
        """Validates result-set order"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY',
                'social',
                'MATCH (ME:Roi)-[friend]->(f) RETURN f.age ORDER BY f.age DESC')

            for i in range(len(result_set)-2):
                age = int(float(result_set[i]))
                next_age = int(float(result_set[i+1]))
                self.assertGreaterEqual(age, next_age)

    def test_limited_order(self):
        """Validates result-set order"""
        with self.redis() as r:
            self.create_graph(r)

            result_set = r.execute_command(
                'graph.QUERY',
                'social',
                'MATCH (ME:Roi)-[friend]->(f) RETURN f.age ORDER BY f.age LIMIT 2')

            # additional record for run time stats
            self.assertEquals(len(result_set), 3)

            for i in range(len(result_set)-2):
                age = int(float(result_set[i]))
                next_age = int(float(result_set[i+1]))
                self.assertLessEqual(age, next_age)

    def test_distinct(self):
        """Validates distinct resultset"""
        with self.redis() as r:
            r.flushdb()
            r.hmset('Roi', {'age':'32', 'name': 'Roi'})
            r.hmset('Alon', {'age': '32', 'name': 'Alon'})
            r.hmset('Omri', {'age': '33', 'name': 'Omri'})
            r.hmset('Boaz', {'age': '31', 'name': 'Boaz'})

            self.assertOk(r.execute_command(
                'graph.ADDEDGE', 'social', 'Roi', 'friend', 'Alon'))
            self.assertOk(r.execute_command(
                'graph.ADDEDGE', 'social', 'Alon', 'friend', 'Omri'))
            self.assertOk(r.execute_command(
                'graph.ADDEDGE', 'social', 'Boaz', 'friend', 'Omri'))

            result_set = r.execute_command(
                'graph.QUERY',
                'social',
                'MATCH (ME:Roi)-[friend]->(f)-[friend]->(fof) RETURN DISTINCT f.name')

            self.assertEquals(len(result_set), 2)

if __name__ == '__main__':
    unittest.main()
