import timeit
from prettytable import PrettyTable

def ExecuteQuery(r, query, graph, qDesc):
    print "%s\n" % qDesc
    print "Query: %s\n" % query

    start = timeit.default_timer()
    resultset = r.execute_command("GRAPH.QUERY", graph, query)
    elapsed = timeit.default_timer() - start
    elapsed_ms = elapsed * 1000
    columns = resultset[0].split(",")
    resultset = resultset[1:]

    tbl = PrettyTable(columns)

    for idx, result in enumerate(resultset):
        if idx < len(resultset)-1:
            tbl.add_row(result.split(","))

    print tbl

    # Last record holds internal execution time.
    print resultset[len(resultset)-1]
    print "Query executed in %.5f miliseconds\n" % elapsed_ms
