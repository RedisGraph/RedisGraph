The automated benchmark definitions `included within tests/benchmarks` folder, provides a framework for evaluating and comparing feature branches and catching regressions prior letting them into the master branch.


To be able to run local/remote you need `redisbench_admin>=0.1.49` . You can install it via PyPi as any other package. 

```
pip install redisbench_admin>=0.1.49
```

Additionally, to run the benchmark locally, you also need the `redisgraph-benchmark-go` tool installed. For a detailed explanation on how to do so, please check [redisgraph-benchmark-go#installation](https://github.com/RedisGraph/redisgraph-benchmark-go#installation).


# Benchmark definition

Each benchmark requires a benchmark definition yaml file to present on the current directory. 
A benchmark definition will then consist of:

- optional db configuration (`dbconfig`) with the proper dataset definition. If no db config is passed then no dataset is loaded during the system setup.

- mandatory client configuration (`clientconfig`) specifing the parameters to pass to the `redisgraph-benchmark-go` tool.

- optional ci/remote definition (`ci`), with the proper terraform deployment configurations definition.

- optional KPIs definition (`kpis`), specifying the target upper or lower bounds for each relevant performance metric. If specified the KPIs definitions constraints the tests passing/failing.

- optional metric exporters definition ( `exporter`: currently only `redistimeseries`), specifying which metrics to parse after each benchmark run and push to remote stores.

Sample benchmark definition:
```yml
name: "UPDATE-BASELINE"
remote:
  - setup: redisgraph-r5
  - type: oss-standalone
dbconfig:
  - dataset: "datasets/single_node.rdb"
clientconfig:
  - tool: redisgraph-benchmark-go
  - parameters:
    - graph: "g"
    - rps: 0
    - clients: 32
    - threads: 4
    - connections: 32
    - requests: 1000000
    - queries:
      - { q: "MATCH (n) WHERE ID(n) = 0 SET n.v = n.v + 1", ratio: 1 }
kpis:
  - le: { $.OverallClientLatencies.Total.q50: 2.0 }
  - ge: { $.OverallQueryRates.Total: 18000 }
  - eq: { $.Totals.Total.Errors: 0 }
exporter:
  redistimeseries:
    timemetric: "$.StartTime"
    metrics:
      - "$.OverallClientLatencies.Total.q50"
      - "$.OverallClientLatencies.Total.q95"
      - "$.OverallClientLatencies.Total.q99"
      - "$.OverallClientLatencies.Total.avg"
      - "$.OverallGraphInternalLatencies.Total.q50"
      - "$.OverallGraphInternalLatencies.Total.q95"
      - "$.OverallGraphInternalLatencies.Total.q99"
      - "$.OverallGraphInternalLatencies.Total.avg"
      - "$.OverallQueryRates.Total"
```

# Running benchmarks

The benchmark automation currently allows running benchmarks in various environments:

- completely locally, if the framework is supported on the local system.

- on AWS, distributing the tasks to multiple EC2 instances as defined on each benchmark specification. To run a benchmark on AWS you additionally need to have a configured AWS account. The application is using the boto3 Python package to exchange files through S3 and create EC2 instances. Triggering of this type of benchmarks can be done from a local machine or via CI on each push to the repo. The results visualization utilities and credentials should have been provided to each team member.

## Run benchmarks locally

To run a benchmark locally call the `make benchmark` rule.

## Run benchmarks remotely

To run a benchmark remotely call  `make benchmark REMOTE=1`.

Some considerations:
- To run a benchmark on AWS you additionally need to have a configured AWS account. You can easily configure it by having the `AWS_ACCESS_KEY_ID`, `AWS_DEFAULT_REGION`, `AWS_SECRET_ACCESS_KEY` variables set.
- You are required to have EC2 instances private key used to connect to the created EC2 instances set via the `EC2_PRIVATE_PEM` environment variable. 
- The git sha, git actor, git org, git repo, and git branch information are required to properly deploy the required EC2 instances. By default that information will be automatically retrieved and can be override by passing the corresponding arguments. 
- Apart from relying on a configured AWS account, the remote benchmarks require terraform to be installed on your local system. Within `./remote/install_deps.sh` you find automation to easily install terraform on linux systems.
- Optionally, at the end of each remote benchmark you push the results json file to the `ci.benchmarks.redislabs` S3 bucket. The pushed results will have a public read ACL. 
- Optionally, at the end of each remote benchmark you can chose the export the key metrics of the benchmark definition to a remote storage like RedisTimeSeries. To do so, you will need the following env variables defined (`PERFORMANCE_RTS_AUTH`, `PERFORMANCE_RTS_HOST`, `PERFORMANCE_RTS_PORT`) or to pass the corresponding arguments.
- By default all benchmark definitions will be run.
- Each benchmark definition will spawn one or multiple EC2 instances as defined on each benchmark specification 
a standalone redis-server, copy the dataset and module files to the DB VM and make usage of redisgraph-benchmark-go to run the query variations. 
- After each benchmark the defined KPIs limits are checked and will influence the exit code of the runner script. Even if we fail a benchmark variation, all other benchmark definitions are run.
- At the end of each benchmark an output json file is stored with this benchmarks folder and will be named like `<start time>-<deployment type>-<git org>-<git repo>-<git branch>-<test name>-<git sha>.json`
- In the case of an uncaught exception after we've deployed the environment the benchmark script will always try to teardown the created environment.