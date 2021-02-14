import argparse
import datetime as dt
import json
import logging
import os
import pathlib
import socket
import sys

import yaml
from jsonpath_ng import parse
from python_terraform import Terraform
from redisbench_admin.utils.redisgraph_benchmark_go import (
    spinUpRemoteRedis,
    setupRemoteBenchmark,
    runRemoteBenchmark,
)
from redisbench_admin.utils.remote import (
    extract_git_vars,
    validateResultExpectations,
    upload_artifacts_to_s3,
    setupRemoteEnviroment,
    checkAndFixPemStr,
    get_run_full_filename,
    fetchRemoteSetupFromConfig,
    pushDataToRedisTimeSeries,
    extractPerBranchTimeSeriesFromResults,
    extractRedisGraphVersion,
    extractPerVersionTimeSeriesFromResults,
)

from redisbench_admin.utils.benchmark_config import (
    parseExporterMetricsDefinition,
    parseExporterTimeMetricDefinition,
    parseExporterTimeMetric,
)

from redistimeseries.client import Client

# Notes
# exporter code specified either in test yml or default.yml
# add ability to run specific benchmark
# timer to deployment
# specify a test

## client
# change the benchmark runner to have ratio on fraction
# export average

# build --- test --- performance
#       --- memcheck

# logging settings
logging.basicConfig(
    format="%(asctime)s %(levelname)-4s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)

# internal aux vars
redisbenchmark_go_link = "https://s3.amazonaws.com/benchmarks.redislabs/redisgraph/redisgraph-benchmark-go/unstable/redisgraph-benchmark-go_linux_amd64"
remote_dataset_file = "/tmp/dump.rdb"
remote_module_file = "/tmp/redisgraph.so"
local_results_file = "./benchmark-result.json"
remote_results_file = "/tmp/benchmark-result.json"
private_key = "/tmp/benchmarks.redislabs.redisgraph.pem"

# environment variables
PERFORMANCE_RTS_AUTH = os.getenv("PERFORMANCE_RTS_AUTH", None)
PERFORMANCE_RTS_HOST = os.getenv("PERFORMANCE_RTS_HOST", 6379)
PERFORMANCE_RTS_PORT = os.getenv("PERFORMANCE_RTS_PORT", None)
TERRAFORM_BIN_PATH = os.getenv("TERRAFORM_BIN_PATH", "terraform")
EC2_ACCESS_KEY = os.getenv("AWS_ACCESS_KEY_ID", None)
EC2_REGION = os.getenv("AWS_DEFAULT_REGION", None)
EC2_SECRET_KEY = os.getenv("AWS_SECRET_ACCESS_KEY", None)
EC2_PRIVATE_PEM = os.getenv("EC2_PRIVATE_PEM", None)

parser = argparse.ArgumentParser(
    description="RedisGraph remote performance tester.",
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
parser.add_argument(
    "--test",
    type=str,
    default="",
    help="Specify a test to run. By default will run all of them.",
)
parser.add_argument("--github_actor", type=str, default=None)
parser.add_argument("--github_repo", type=str, default=None)
parser.add_argument("--github_org", type=str, default=None)
parser.add_argument("--github_sha", type=str, default=None)
parser.add_argument("--github_branch", type=str, default=None)
parser.add_argument("--triggering_env", type=str, default=socket.gethostname())
parser.add_argument("--terraform_bin_path", type=str, default=TERRAFORM_BIN_PATH)
parser.add_argument("--module_path", type=str, default="./../../src/redisgraph.so")
parser.add_argument("--setup_name_sufix", type=str, default="")
parser.add_argument(
    "--s3_bucket_name",
    type=str,
    default="ci.benchmarks.redislabs",
    help="S3 bucket name.",
)
parser.add_argument(
    "--upload_results_s3",
    default=False,
    action="store_true",
    help="uploads the result files and configuration file to public ci.benchmarks.redislabs bucket. Proper credentials are required",
)
parser.add_argument("--redistimesies_host", type=str, default=PERFORMANCE_RTS_HOST)
parser.add_argument("--redistimesies_port", type=int, default=PERFORMANCE_RTS_PORT)
parser.add_argument("--redistimesies_pass", type=str, default=PERFORMANCE_RTS_AUTH)
parser.add_argument(
    "--push_results_redistimeseries",
    default=False,
    action="store_true",
    help="uploads the results to RedisTimeSeries. Proper credentials are required",
)
parser.add_argument(
    "--skip-env-vars-verify",
    default=False,
    action="store_true",
    help="skip environment variables check",
)

args = parser.parse_args()

tf_bin_path = args.terraform_bin_path
tf_github_org = args.github_org
tf_github_actor = args.github_actor
tf_github_repo = args.github_repo
tf_github_sha = args.github_sha
tf_github_branch = args.github_branch

if tf_github_actor is None:
    (
        github_org_name,
        github_repo_name,
        github_sha,
        github_actor,
        github_branch,
    ) = extract_git_vars()
    tf_github_org = github_org_name
if tf_github_actor is None:
    (
        github_org_name,
        github_repo_name,
        github_sha,
        github_actor,
        github_branch,
    ) = extract_git_vars()
    tf_github_actor = github_actor
if tf_github_repo is None:
    (
        github_org_name,
        github_repo_name,
        github_sha,
        github_actor,
        github_branch,
    ) = extract_git_vars()
    tf_github_repo = github_repo_name
if tf_github_sha is None:
    (
        github_org_name,
        github_repo_name,
        github_sha,
        github_actor,
        github_branch,
    ) = extract_git_vars()
    tf_github_sha = github_sha
if tf_github_branch is None:
    (
        github_org_name,
        github_repo_name,
        github_sha,
        github_actor,
        github_branch,
    ) = extract_git_vars()
    tf_github_branch = github_branch

tf_triggering_env = args.triggering_env
tf_setup_name_sufix = "{}-{}".format(args.setup_name_sufix, tf_github_sha)
s3_bucket_name = args.s3_bucket_name
local_module_file = args.module_path

if args.skip_env_vars_verify is False:
    if EC2_ACCESS_KEY is None or EC2_ACCESS_KEY == "":
        logging.error("missing required AWS_ACCESS_KEY_ID env variable")
        exit(1)
    if EC2_REGION is None or EC2_REGION == "":
        logging.error("missing required AWS_DEFAULT_REGION env variable")
        exit(1)
    if EC2_SECRET_KEY is None or EC2_SECRET_KEY == "":
        logging.error("missing required AWS_SECRET_ACCESS_KEY env variable")
        exit(1)

if EC2_PRIVATE_PEM is None or EC2_PRIVATE_PEM == "":
    logging.error("missing required EC2_PRIVATE_PEM env variable")
    exit(1)

logging.info("Using the following vars on terraform deployment:")
logging.info("\tterraform bin path: {}".format(tf_bin_path))
logging.info("\tgithub_actor: {}".format(tf_github_actor))
logging.info("\tgithub_org: {}".format(tf_github_org))
logging.info("\tgithub_repo: {}".format(tf_github_repo))
logging.info("\tgithub_branch: {}".format(tf_github_branch))
logging.info("\tgithub_sha: {}".format(tf_github_sha))
logging.info("\ttriggering env: {}".format(tf_triggering_env))
logging.info("\tprivate_key path: {}".format(private_key))
logging.info("\tsetup_name sufix: {}".format(tf_setup_name_sufix))


with open(private_key, "w") as tmp_private_key_file:
    pem_str = checkAndFixPemStr(EC2_PRIVATE_PEM)
    tmp_private_key_file.write(pem_str)

return_code = 0

files = []
if args.test == "":
    files = pathlib.Path().glob("*.yml")
    files = [str(x) for x in files]
    files.remove("defaults.yml")
    logging.info(
        "Running all specified benchmarks: {}".format(" ".join([str(x) for x in files]))
    )
else:
    logging.info("Running specific benchmark in file: {}".format(args.test))
    files = [args.test]

default_metrics = []
exporter_timemetric_path = None
defaults_filename = "defaults.yml"
with open(defaults_filename, "r") as stream:
    logging.info(
        "Loading default specifications from file: {}".format(defaults_filename)
    )
    default_config = yaml.safe_load(stream)
    if "exporter" in default_config:
        default_metrics = parseExporterMetricsDefinition(default_config["exporter"])
        if len(default_metrics) > 0:
            logging.info(
                "Found RedisTimeSeries default metrics specification. Will include the following metrics on all benchmarks {}".format(
                    " ".join(default_metrics)
                )
            )
        exporter_timemetric_path = parseExporterTimeMetricDefinition(
            default_config["exporter"]
        )
        if exporter_timemetric_path is not None:
            logging.info(
                "Found RedisTimeSeries default time metric specification. Will use the following JSON path to retrieve the test time {}".format(
                    exporter_timemetric_path
                )
            )


for f in files:
    with open(f, "r") as stream:
        benchmark_config = yaml.safe_load(stream)
        test_name = benchmark_config["name"]
        s3_bucket_path = "{github_org}/{github_repo}/results/{test_name}/".format(
            github_org=tf_github_org, github_repo=tf_github_repo, test_name=test_name
        )
        s3_uri = "https://s3.amazonaws.com/{bucket_name}/{bucket_path}".format(
            bucket_name=s3_bucket_name, bucket_path=s3_bucket_path
        )

        if "remote" in benchmark_config:
            remote_setup, deployment_type = fetchRemoteSetupFromConfig(
                benchmark_config["remote"]
            )
            logging.info(
                "Deploying test defined in {} on AWS using {}".format(f, remote_setup)
            )
            tf_setup_name = "{}{}".format(remote_setup, tf_setup_name_sufix)
            logging.info("Using full setup name: {}".format(tf_setup_name))
            # check if terraform is present
            tf = Terraform(
                working_dir=remote_setup,
                terraform_bin_path=tf_bin_path,
            )
            (
                return_code,
                username,
                server_private_ip,
                server_public_ip,
                server_plaintext_port,
                client_private_ip,
                client_public_ip,
            ) = setupRemoteEnviroment(
                tf,
                tf_github_sha,
                tf_github_actor,
                tf_setup_name,
                tf_github_org,
                tf_github_repo,
                tf_triggering_env,
            )
            # after we've created the env, even on error we should always teardown
            # in case of some unexpected error we fail the test
            try:
                # setup RedisGraph
                spinUpRemoteRedis(
                    benchmark_config,
                    server_public_ip,
                    username,
                    private_key,
                    local_module_file,
                    remote_module_file,
                    remote_dataset_file,
                )

                # setup the benchmark
                setupRemoteBenchmark(
                    client_public_ip, username, private_key, redisbenchmark_go_link
                )
                start_time = dt.datetime.now()
                start_time_str = start_time.strftime("%Y-%m-%d-%H-%M-%S")
                local_benchmark_output_filename = get_run_full_filename(
                    start_time_str,
                    deployment_type,
                    tf_github_org,
                    tf_github_repo,
                    tf_github_branch,
                    test_name,
                    tf_github_sha,
                )
                logging.info(
                    "Will store benchmark json output to local file {}".format(
                        local_benchmark_output_filename
                    )
                )
                # run the benchmark
                runRemoteBenchmark(
                    client_public_ip,
                    username,
                    private_key,
                    server_private_ip,
                    server_plaintext_port,
                    benchmark_config,
                    remote_results_file,
                    local_benchmark_output_filename,
                )

                # check KPIs
                result = True
                results_dict = None
                with open(local_benchmark_output_filename, "r") as json_file:
                    results_dict = json.load(json_file)

                if "kpis" in benchmark_config:
                    result = validateResultExpectations(
                        benchmark_config, results_dict, result, expectations_key="kpis"
                    )
                    if result is not True:
                        return_code |= 1

                if args.upload_results_s3:
                    logging.info(
                        "Uploading results to s3. s3 bucket name: {}. s3 bucket path: {}".format(
                            s3_bucket_name, s3_bucket_path
                        )
                    )
                    artifacts = [local_benchmark_output_filename]
                    upload_artifacts_to_s3(artifacts, s3_bucket_name, s3_bucket_path)

                if args.push_results_redistimeseries:
                    logging.info("Pushing results to RedisTimeSeries.")
                    rts = Client(
                        host=args.redistimesies_host,
                        port=args.redistimesies_port,
                        password=args.redistimesies_pass,
                    )
                    # check which metrics to extract
                    metrics = default_metrics
                    if "exporter" in benchmark_config:
                        extra_metrics = parseExporterMetricsDefinition(
                            benchmark_config["exporter"]
                        )
                        metrics.extend(extra_metrics)
                        extra_timemetric_path = parseExporterTimeMetricDefinition(
                            benchmark_config["exporter"]
                        )
                        if extra_timemetric_path is not None:
                            exporter_timemetric_path = extra_timemetric_path

                    # extract timestamp
                    datapoints_timestamp = parseExporterTimeMetric(
                        exporter_timemetric_path, results_dict
                    )

                    rg_version = extractRedisGraphVersion(results_dict)

                    # extract per branch datapoints
                    (
                        ok,
                        per_version_time_series_dict,
                    ) = extractPerVersionTimeSeriesFromResults(
                        datapoints_timestamp,
                        metrics,
                        results_dict,
                        rg_version,
                        tf_github_org,
                        tf_github_repo,
                        deployment_type,
                        test_name,
                        tf_triggering_env,
                    )

                    # push per-branch data
                    pushDataToRedisTimeSeries(rts, per_version_time_series_dict)

                    # extract per branch datapoints
                    ok, branch_time_series_dict = extractPerBranchTimeSeriesFromResults(
                        datapoints_timestamp,
                        metrics,
                        results_dict,
                        str(tf_github_branch),
                        tf_github_org,
                        tf_github_repo,
                        deployment_type,
                        test_name,
                        tf_triggering_env,
                    )

                    # push per-branch data
                    pushDataToRedisTimeSeries(rts, branch_time_series_dict)
            except:
                return_code |= 1
                logging.critical(
                    "Some unexpected exception was caught during remote work. Failing test...."
                )
                logging.critical(sys.exc_info()[0])
            finally:
                # tear-down
                logging.info("Tearing down setup")
                tf_output = tf.destroy()
                logging.info("Tear-down completed")

exit(return_code)
