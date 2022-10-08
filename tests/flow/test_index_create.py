from common import *
from pathos.pools import ProcessPool as Pool

GRAPH_ID = "index"
redis_graph = None
redis_con = None

class testIndexCreationFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        global redis_con
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    # full-text index creation
    def test01_fulltext_index_creation(self):
        # create an index over L:v0
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v0')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L:v1
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v1')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L:v1 and L:v2
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v1', 'v2')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L:v0, L:v1 and L:v2
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v0', 'v1', 'v2')")
        self.env.assertEquals(result.indices_created, 0)

        # create an index over L:v2, L:v1 and L:v0
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v2', 'v1', 'v0')")
        self.env.assertEquals(result.indices_created, 0)

        # create an index over L:v3 and L:v4
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v3', 'v4')")
        self.env.assertEquals(result.indices_created, 2)

        # create an index over L:v5 and L:v6
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L' }, 'v5', 'v6')")
        self.env.assertEquals(result.indices_created, 2)

    def test02_fulltext_index_creation_label_config(self):
        # create an index over L1:v1
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L1' }, 'v1')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L1:v2, v3
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L1' }, 'v2', 'v3')")
        self.env.assertEquals(result.indices_created, 2)

        # create an index over L2:v1 with stopwords
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L2', stopwords: ['The'] }, 'v1')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L2:v2
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L2' }, 'v2')")
        self.env.assertEquals(result.indices_created, 1)

        try:
            # try to create an index, without specifying the label
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ stopwords: ['The'] }, 'v4')")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Label is missing", str(e))

        try:
            # create an index over L1:v4 with stopwords should failed
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L1', stopwords: ['The'] }, 'v4')")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Index already exists configuration can't be changed", str(e))

        try:
            # create an index over L1:v4 with language should failed
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L1', language: 'english' }, 'v4')")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Index already exists configuration can't be changed", str(e))

        # drop L1 index
        result = redis_graph.query("CALL db.idx.fulltext.drop('L1')")
        self.env.assertEquals(result.indices_deleted, 1)

        try:
            # create an index over L1:v4 with an unsupported language, expecting to failed
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L1', language: 'x' }, 'v4')")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Language is not supported", str(e))

        # create an index over L1:v4 with language
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L1', language: 'english' }, 'v4')")
        self.env.assertEquals(result.indices_created, 1)

        try:
            # create an index over L3:v1 with stopwords should failed
            # stopwords must be provided as an array of strings
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L3', stopwords: 'The' }, 'v1')")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Stopwords must be array", str(e))

        try:
            # create an index over L3:v1 with language should failed
            # language must be provided as a string
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L3', language: ['english'] }, 'v1')")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Language must be string", str(e))

        try:
            # create an index over L3 should failed, missing field(s)
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L3', { })")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Field is missing", str(e))

        try:
            # create an index over L3:v1 with weight of type string should failed
            # weight must be provided as numeric
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L3', { field: 'v1', weight: '1' })")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Weight must be numeric", str(e))

        try:
            # create an index over L3:v1 with nostem of type string should failed
            # nostem must be boolean
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L3', { field: 'v1', nostem: 'true' })")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Nostem must be bool", str(e))
        
        try:
            # create an index over L3:v1 with phonetic of type bool should failed
            # phonetic must be a string
            result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L3', { field: 'v1', phonetic: true })")
            assert(False)
        except ResponseError as e:
            self.env.assertIn("Phonetic must be a string", str(e))

    def test03_multi_prop_index_creation(self):
        # create an index over person:age and person:name
        result = redis_graph.query("CREATE INDEX ON :person(age, name)")
        self.env.assertEquals(result.indices_created, 2)

        # try to create an index over person:age and person:name, index shouldn't be created as it already exist
        result = redis_graph.query("CREATE INDEX ON :person(age, name)")
        self.env.assertEquals(result.indices_created, 0)

        # try to create an index over person:name and person:age, index shouldn't be created as it already exist
        result = redis_graph.query("CREATE INDEX ON :person(name, age)")
        self.env.assertEquals(result.indices_created, 0)

        # try to create an index over person:age and person:name and person:height, index for height should be created as the rest already exist
        result = redis_graph.query("CREATE INDEX ON :person(age, age, name, height)")
        self.env.assertEquals(result.indices_created, 1)

        # try to create an index over person:gender and person:name and person:height, index for gender should be created as the rest already exist
        result = redis_graph.query("CREATE INDEX ON :person(gender, gender, name, height)")
        self.env.assertEquals(result.indices_created, 1)

    def test04_index_creation_pattern_syntax(self):
        # create an index over user:age and user:name
        result = redis_graph.query("CREATE INDEX FOR (p:user) ON (p.age, p.name)")
        self.env.assertEquals(result.indices_created, 2)

        # create an index over follow:prop1 and follow:prop2
        result = redis_graph.query("CREATE INDEX FOR ()-[r:follow]-() ON (r.prop1, r.prop2)")
        self.env.assertEquals(result.indices_created, 2)

    def test05_index_delete(self):
        def create_drop_index(graph_id):
            env = Env(decodeResponses=True)
            redis_con = env.getConnection()
            for _ in range(1, 100):
                pipe = redis_con.pipeline()
                pipe.execute_command("GRAPH.QUERY", f"x{graph_id}", "CREATE (a:L), (n:L), (n)-[:T]->(a)")
                pipe.execute_command("GRAPH.QUERY", f"x{graph_id}", "CREATE INDEX FOR ()-[n:T]-() ON (n.p)")
                pipe.execute()
                redis_con.execute_command("GRAPH.DELETE", f"x{graph_id}")
        pool = Pool(nodes=10)
        pool.map(create_drop_index, range(1, 100))

    def test06_syntax_error_index_creation(self):
        # create index on invalid property name
        try:
            redis_graph.query("CREATE INDEX FOR (p:Person) ON (p.m.n, p.p.q)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input '.': expected ',' or ')'", str(e))

        # create index on invalid identifier
        try:
            redis_graph.query("CREATE INDEX FOR (p:Person) ON (1.b)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input '1': expected an identifier", str(e))

        # create index on invalid property name: number
        try:
            redis_graph.query("CREATE INDEX FOR (p:Person) ON (b.1)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input '1': expected a property name", str(e))

        # create index without label
        try:
            redis_graph.query("CREATE INDEX FOR (Person) ON (surname)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input ')': expected a label", str(e))

        # create index without property name
        try:
            redis_graph.query("CREATE INDEX FOR (p:Person) ON (surname)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input ')': expected '.'", str(e))

        # create index without identifier
        try:
            redis_graph.query("CREATE INDEX FOR (p:Person) ON ()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input ')': expected an identifier", str(e))

        # create index for relationship on invalid property name
        try:
            redis_graph.query("CREATE INDEX FOR ()-[n:T]-() ON (n.p.q)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input '.': expected ',' or ')'", str(e))

        # create index for relationship on invalid identifier
        try:
            redis_graph.query("CREATE INDEX FOR ()-[n:T]-() ON (1.b)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid input '1': expected an identifier", str(e))

    def test07_index_creation_undefined_identifier(self):   
        # create index on undefined identifier
        try:
            redis_graph.query("CREATE INDEX FOR (p:Person) ON (a.b)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("a not defined", str(e))

        # create index on undefined identifier after defined identifier
        try:
            redis_graph.query("CREATE INDEX FOR (p:Person) ON (p.x, a.b)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("a not defined", str(e))
        
        # create index for relationship on undefined identifier
        try:
            redis_graph.query("CREATE INDEX FOR ()-[n:T]-() ON (a.b)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("a not defined", str(e))

        # create index for relationship on undefined identifier after defined identifier
        try:
            redis_graph.query("CREATE INDEX FOR ()-[n:T]-() ON (p.x, a.b)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("a not defined", str(e))
