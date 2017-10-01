import redis
from prettytable import PrettyTable

class Node(object):
    """
    """
    def __init__(self, node_id, *args, **kwargs):
        """
        """
        self.id = node_id
        self.label = kwargs["label"]
        self.properties = {}

        args = list(args)
        for i in range(0, len(args), 2):
            self.properties[args[i]] = args[i+1]

class Edge(object):
    """
    """
    def __init__(self, relation, *args):
        """
        """
        self.relation = relation
        self.properties = {}

        args = list(args)
        for i in range(0, len(args), 2):
            self.properties[args[i]] = args[i+1]

class RedisGraph(object):
    """
    """
    def __init__(self, graph, redis_con):
        """
        """
        self.graph = graph
        self.redis_con = redis_con

    def create_node(self, *args, **kwargs):
        """
        Creates a new node, specify node properties
        within args where args[i] i%2=0 is
        the prop name, and i%2=1 is prop value
        """
        label = kwargs["label"]
        if label:
            node_id = self.redis_con.execute_command(
                "GRAPH.CREATENODE",
                self.graph,
                label,
                *args)
        else:
            node_id = self.redis_con.execute_command(
                "GRAPH.CREATENODE",
                self.graph,
                *args)

        return Node(node_id, label=label, *args)

    def connect_nodes(self, src_node, relation, dest_node, *args):
        """
        Connects source node to dest node with relation
        specify relation properties within
        args where args[i] i%2=0 is
        the prop name, and i%2=1 is prop value
        """
        resp = self.redis_con.execute_command(
            "GRAPH.ADDEDGE",
            self.graph,
            src_node.id,
            dest_node.id,
            relation,
            *args)

        if resp == "OK":
            return Edge(relation, *args)
        else:
            return None

    def query(self, q):
        """
        """
        resultset = self.redis_con.execute_command("GRAPH.QUERY", self.graph, q)
        columns = resultset[0].split(",")
        resultset = resultset[1:]
        tbl = PrettyTable(columns)

        for idx, result in enumerate(resultset):
            if idx < len(resultset)-1:
                tbl.add_row(result.split(","))

        print tbl
        # Last record holds internal execution time.
        print resultset[len(resultset)-1]
        return tbl

    def execution_plan(self, query):
        return self.redis_con.execute_command("GRAPH.EXPLAIN", self.graph, query)
