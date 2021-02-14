import argparse
import datetime as dt
import json
import logging
import pathlib
import subprocess

import yaml
from redisbench_admin.utils.local import (
    spinUpLocalRedis,
    prepareSingleBenchmarkCommand,
    getLocalRunFullFilename,
    isProcessAlive,
)
from redisbench_admin.utils.remote import (
    extract_git_vars,
    validateResultExpectations,
)

# logging settings
logging.basicConfig(
    format="%(asctime)s %(levelname)-4s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)

parser = argparse.ArgumentParser(
    description="RedisGraph remote performance tester.",
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)

parser.add_argument("--module_path", type=str, default="./../../src/redisgraph.so")
parser.add_argument(
    "--test",
    type=str,
    default="",
    help="specify a test to run. By default will run all of them.",
)
parser.add_argument("--redisgraph_port", type=int, default=6379)
args = parser.parse_args()

(
    github_org_name,
    github_repo_name,
    github_sha,
    github_actor,
    github_branch,
) = extract_git_vars()

local_module_file = args.module_path

logging.info("Retrieved the following local info:")
logging.info("\tgithub_actor: {}".format(github_actor))
logging.info("\tgithub_org: {}".format(github_org_name))
logging.info("\tgithub_repo: {}".format(github_repo_name))
logging.info("\tgithub_branch: {}".format(github_branch))
logging.info("\tgithub_sha: {}".format(github_sha))

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

for f in files:
    with open(f, "r") as stream:
        redis_process = None
        benchmark_config = yaml.safe_load(stream)
        test_name = benchmark_config["name"]
        # after we've spinned Redis, even on error we should always teardown
        # in case of some unexpected error we fail the test
        try:
            # setup RedisGraph
            redis_process = spinUpLocalRedis(
                benchmark_config,
                args.redisgraph_port,
                local_module_file,
            )
            if isProcessAlive(redis_process) is False:
                raise Exception("Redis process is not alive. Failing test.")
            # setup the benchmark
            start_time = dt.datetime.now()
            start_time_str = start_time.strftime("%Y-%m-%d-%H-%M-%S")
            local_benchmark_output_filename = getLocalRunFullFilename(
                start_time_str,
                github_branch,
                test_name,
            )
            logging.info(
                "Will store benchmark json output to local file {}".format(
                    local_benchmark_output_filename
                )
            )

            command = prepareSingleBenchmarkCommand(
                "redisgraph-benchmark-go",
                "localhost",
                args.redisgraph_port,
                benchmark_config,
                local_benchmark_output_filename,
            )

            # run the benchmark
            redisgraph_benchmark_go_process = subprocess.Popen(args=command)
            result = redisgraph_benchmark_go_process.communicate()
            logging.info("Extracting the benchmark results")

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
        except:
            raise
            return_code |= 1
            logging.critical(
                "Some unexpected exception was caught during remote work. Failing test...."
            )
            logging.critical(sys.exc_info()[0])
    # tear-down
    logging.info("Tearing down setup")
    if redis_process is not None:
        redis_process.kill()
    logging.info("Tear-down completed")

exit(return_code)
