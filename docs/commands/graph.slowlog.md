Returns a list containing up to 10 of the slowest queries issued against the given graph ID.

Each item in the list has the following structure:

1. A Unix timestamp at which the log entry was processed.
2. The issued command.
3. The issued query.
4. The amount of time needed for its execution, in milliseconds.

```sh
GRAPH.SLOWLOG graph_id
 1) 1) "1581932396"
    2) "GRAPH.QUERY"
    3) "MATCH (a:Person)-[:FRIEND]->(e) RETURN e.name"
    4) "0.831"
 2) 1) "1581932396"
    2) "GRAPH.QUERY"
    3) "MATCH (me:Person)-[:FRIEND]->(:Person)-[:FRIEND]->(fof:Person) RETURN fof.name"
    4) "0.288"
```

To reset a graph's slowlog issue the following command:

```sh
GRAPH.SLOWLOG graph_id RESET
```

Once cleared the information is lost forever.
