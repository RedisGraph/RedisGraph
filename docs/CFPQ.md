# Quick start: Context-Free Path Querying (CFPQ) in RedisGraph

## Prerequisites
To try to execute CFQP in RedisGraph you only need to run docker container:

```bash
docker run -p 6379:6379 -it --rm simpletondl/redisgraph:latest
127.0.0.1:6379>
```

 [This docker image](https://hub.docker.com/r/simpletondl/redisgraph) already contains some graph _company_ described below, and also runs the redis-cli at startup. So you can immediately make queries in the shell, f.e. count the number of vertices:

```
127.0.0.1:6379> GRAPH.QUERY company "MATCH () RETURN COUNT(*)"
```

## Context-Free Path Querying (CFPQ)

CFPQ is a way to use context-free grammars as paths constraints by the same way as one can use regular expressions to specify sequences which can be formed by paths labels.

One of the classical example of context-free query is a *same-generation query* which can be expressed in terms of context-free grammar as follows:
```
s -> X X_R | X s X_R
```
where ```X``` is a some relation and ```X_R``` is a reversed relation. 
Thus, CFPQ allows one to explore hierarchical patterns in graph-structured data.

Let, for example, consider the following database which represents inner structure of some company.

![Company hierarchy](/docs/images/cfpq_example.dot.svg "The hierarchy of the company")

Suppose one wants to find all employers who have the same position in the company but have different salaries. To do it one can use the following Cypher query.

```
PATH PATTERN OnSamePosition  = ()-/ :boss> [~OnSamePosition | ()] <:boss /->()
MATCH (a)-/ ~OnSamePosition /->(b)
WHERE a.salary <> b.salary
RETURN a.name, b.name
```

You can run it inside a docker container:

```
127.0.0.1:6379> GRAPH.QUERY company "PATH PATTERN OnSamePosition  = ()-/ :boss> [~OnSamePosition | ()] <:boss /->() MATCH (a)-/ ~OnSamePosition /->(b) WHERE a.salary <> b.salary RETURN a.name, b.name"
```

Here named path pattern ```OnSamePosition``` is used to specify connection between vertices are on the same level of hierarchy over ```boss``` relation, and it is a context-free pattern.

To find who is in a position lower than Tim and has a salary higher than Tim one can use the following query.

```
PATH PATTERN OnSamePosition = ()-/ :boss [~OnSamePosition | ()] <:boss  /->()
MATCH (a)-/ ~OnSamePosition /->(b)<-[boss*1..]-(c)
WHERE a.name = 'Tim' and a.salary < c.salary
RETURN a.name, a.salary, c.name, c.salary
```

Inside docker container:
```
127.0.0.1:6379> GRAPH.QUERY company "PATH PATTERN OnSamePosition = ()-/ :boss [~OnSamePosition | ()] <:boss  /->() MATCH (a)-/ ~OnSamePosition /->(b)<-[boss*1..]-(c) WHERE a.name = 'Tim' and a.salary < c.salary RETURN a.name, a.salary, c.name, c.salary"
```

To summarize, CFPQ can be use in different areas for graph-structured data analysis. Some examples of CFQP applications are listed below.
- Static code analysis. Thus, one can use graph database as a backend for static code analysis systems.
  - Taint analysis: [Scalable and Precise Taint Analysis for Android](http://huangw5.github.io/docs/issta15.pdf) 
  - Points-to analysis/alias analysis:
     - [An Incremental Points-to Analysis with CFL-Reachability](https://www.researchgate.net/publication/262173734_An_Incremental_Points-to_Analysis_with_CFL-Reachability)
     - [Graspan: A Single-machine Disk-based Graph System for Interprocedural Static Analyses of Large-scale Systems Code](https://dl.acm.org/doi/10.1145/3037697.3037744)
  - Binding time analysis: [BTA Termination Using CFL-Reachability](https://www.researchgate.net/publication/2467654_BTA_Termination_Using_CFL-Reachability)
- Graph segmentation: [Understanding Data Science Lifecycle Provenance via Graph Segmentation and Summarization](https://ieeexplore.ieee.org/abstract/document/8731467)
- Biological data analysis: [Subgraph queries by context-free grammars](https://www.researchgate.net/publication/321662505_Subgraph_Queries_by_Context-free_Grammars)
