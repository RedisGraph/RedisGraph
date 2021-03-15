
# Goals 
The RedisGraph automated benchmark definitions provide a framework for evaluating and comparing feature branches and catching regressions prior letting them into the master branch. 

# Benchmark definition

Each benchmark requires a benchmark definition yaml file to present on the current directory. 
A benchmark definition will then consist of:

- optional db configuration (`dbconfig`) with the proper dataset definition. If no db config is passed then no dataset is loaded during the system setup.

- mandatory client configuration (`clientconfig`) specifing the parameters to pass to the `redisgraph-benchmark-go` tool.

- mandatory queries definition (`queries`) specifying which queries, and at what ratio the benchmark should issue.

- optional ci/remote definition (`ci`), with the proper terraform deployment configurations definition.

- optional KPIs definition (`kpis`), specifying the target upper or lower bounds for each relevant performance metric. If specified the KPIs definitions constraints the tests passing/failing.

- optional metric exporters definition (currently only `redistimeseries`), specifying which metrics to parse after each benchmark run and push to remote stores. Currently we export the redistimeseries metrics broken by version, and by branch.

Sample benchmark definition:
```yml
name: "UPDATE-BASELINE"
remote:
  - setup: redisgraph-r5
  - type: oss-standalone
dbconfig:
  - dataset: "datasets/single_node.rdb"
clientconfig:
  - graph: "g"
  - rps: 0
  - clients: 32
  - threads: 4
  - connections: 32
  - requests: 1000000
queries:
  - { q: "MATCH (n) WHERE ID(n) = 0 SET n.v = n.v + 1", ratio: 1 }
kpis:
  - le: { $.OverallClientLatencies.Total.q50: 1.5 }
  - ge: { $.OverallQueryRates.Total: 20000 }
```

## Common benchmark definition

There are common parts of a benchmark definition that are valid across benchmarks. To ease the benchmark definition process, and to reduce the duplicate yaml among benchmarks we've added a special yml file named `defaults.yml` that will be used across all benchmark definitions. Currently, only the exporter definition is propagated across all benchmarks, as follow:

```yml
version: 0.1
kpis:
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

- on AWS, distributing the tasks to multiple EC2 instances as defined on each benchmark specification. To run a benchmark on AWS you additionally need to have a configured AWS account. The application is using the boto3 Python package to exchange files through S3 and create EC2 instances. Triggering of this type of benchmarks can be done from a local machine or via CI on each push to the repo. The results visualization utilities and credentials should have been provide to each team member.

# Run benchmarks locally 

To run a benchmark locally call the `local-runner.py` from within the benchmarks folder, or use the benchmark Makefile rule from within the project root folder: `make benchmark-local`.

Some considerations:
- By default all benchmark definitions will be run.
- By default the local-runner script assumes you have a compiled `redisgraph.so` module within src folder. 
- Each benchmark definition will spawn a standalone redis-server and make usage of redisgraph-benchmark-go to run the query variations. 
- After each benchmark the defined KPIs limits are checked and will influence the exit code of the runner script.
- At the end of each benchmark an output json file is stored with this benchamrks folder and will be named like `<start time>-<git branch>-<test name>.json`

For the complete list of supported local arguments, run:

python3 local-runner.py --help

```
$ python3 local-runner.py --help
usage: local-runner.py [-h] [--module_path MODULE_PATH] [--redisgraph_port REDISGRAPH_PORT]

RedisGraph remote performance tester.

optional arguments:
  -h, --help            show this help message and exit
  --module_path MODULE_PATH
  --redisgraph_port REDISGRAPH_PORT
```


# Run benchmarks remotely on steady stable VMs with sustained performance

To run a benchmark remotely call the `remote-runner.py`. 
Some considerations:
- To run a benchmark on AWS you additionally need to have a configured AWS account. You can easily configure it by having the `AWS_ACCESS_KEY_ID`, `AWS_DEFAULT_REGION`, `AWS_SECRET_ACCESS_KEY` variables set.
- You are required to have EC2 instances private key used to connect to the created EC2 instances set via the `EC2_PRIVATE_PEM` environment variable. 
- The git sha, git actor, git org, git repo, and git branch information are required to properly deploy the required EC2 instances. By default that information will be automatically retrieved and can be override by passing the corresponding arguments. 
- Apart from relying on a configured AWS account, the remote benchmarks require terraform to be installed on your local system. Within `./remote/install_deps.sh` you find automation to easily install terraform on linux systems.
- Optionally, at the end of each remote benchmark you push the results json file to the `ci.benchmarks.redislabs` S3 bucket. The pushed results will have a public read ACL. 
- Optionally, at the end of each remote benchmark you can chose the export the key metrics of the benchmark definition to a remote storage like RedisTimeSeries. To do so, you will need the following env variables defined (`PERFORMANCE_RTS_AUTH`, `PERFORMANCE_RTS_HOST`, `PERFORMANCE_RTS_PORT`) or to pass the corresponding arguments.
- By default all benchmark definitions will be run.
- By default the remote-runner script assumes you have a compiled `redisgraph.so` module within src folder. 
- Each benchmark definition will spawn one or multiple EC2 instances as defined on each benchmark specification 
a standalone redis-server, copy the dataset and module files to the DB VM and make usage of redisgraph-benchmark-go to run the query variations. 
- After each benchmark the defined KPIs limits are checked and will influence the exit code of the runner script. Even if we fail a benchmark variation, all other benchmark definitions are run.
- At the end of each benchmark an output json file is stored with this benchmarks folder and will be named like `<start time>-<deployment type>-<git org>-<git repo>-<git branch>-<test name>-<git sha>.json`
- In the case of a uncaught exception after we've deployed the environment the benchamrk script will always try to teardown the created environment. 

For the complete list of supported remote run arguments, run:

python3 remote-runner.py --help

```
$ python3 remote-runner.py --help
usage: remote-runner.py [-h] [--github_actor GITHUB_ACTOR] [--github_repo GITHUB_REPO] [--github_org GITHUB_ORG] [--github_sha GITHUB_SHA] [--github_branch GITHUB_BRANCH]
                        [--triggering_env TRIGGERING_ENV] [--terraform_bin_path TERRAFORM_BIN_PATH] [--module_path MODULE_PATH] [--setup_name_sufix SETUP_NAME_SUFIX] [--s3_bucket_name S3_BUCKET_NAME]
                        [--upload_results_s3] [--redistimesies_host REDISTIMESIES_HOST] [--redistimesies_port REDISTIMESIES_PORT] [--redistimesies_pass REDISTIMESIES_PASS]
                        [--push_results_redistimeseries] [--skip-env-vars-verify]

RedisGraph remote performance tester.

optional arguments:
  -h, --help            show this help message and exit
  --github_actor GITHUB_ACTOR
  --github_repo GITHUB_REPO
  --github_org GITHUB_ORG
  --github_sha GITHUB_SHA
  --github_branch GITHUB_BRANCH
  --triggering_env TRIGGERING_ENV
  --terraform_bin_path TERRAFORM_BIN_PATH
  --module_path MODULE_PATH
  --setup_name_sufix SETUP_NAME_SUFIX
  --s3_bucket_name S3_BUCKET_NAME
                        S3 bucket name. (default: ci.benchmarks.redislabs)
  --upload_results_s3   uploads the result files and configuration file to public ci.benchmarks.redislabs bucket. Proper credentials are required (default: False)
  --redistimesies_host REDISTIMESIES_HOST
  --redistimesies_port REDISTIMESIES_PORT
  --redistimesies_pass REDISTIMESIES_PASS
  --push_results_redistimeseries
                        uploads the results to RedisTimeSeries. Proper credentials are required (default: False)
  --skip-env-vars-verify
                        skip environment variables check (default: False)
```

# Locally comparing two distinct runs

TBD
