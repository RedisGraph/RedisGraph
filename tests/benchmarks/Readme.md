# Context

The automated benchmark definitions included within `tests/benchmarks` folder, provides a framework for evaluating and comparing feature branches and catching regressions prior to letting them into the master branch.

To be able to run local benchmarks you need:
-  `redisbench_admin>=0.3.11` [[tool repo for full details](https://github.com/RedisLabsModules/redisbench-admin)]. You can install `redisbench-admin` via PyPi as any other package.
   ```
    pip3 install redisbench_admin>=0.3.11
    ``` 
-  redis-server installed locally
-  the benchmark tools specified on each configuration file.  For `redisgraph-benchmark-go` [[tool repo for full details](https://github.com/RedisGraph/redisgraph-benchmark-go)] you can check bellow a quick way of downloading the prebuilt binaries for "linux/amd64" and "darwin/amd64".


### redisgraph-benchmark-go installation

If you don't have Go on your machine and just want to use the produced binaries you can download Linux x86, and Darwin x86 from the following links:

- linux/amd64: [redisgraph-benchmark-go_linux_amd64](https://s3.amazonaws.com/benchmarks.redislabs/redisgraph/tools/redisgraph-benchmark-go/redisgraph-benchmark-go_linux_amd64)

- darwin/amd64: [redisgraph-benchmark-go_darwin_amd64](https://s3.amazonaws.com/benchmarks.redislabs/redisgraph/tools/redisgraph-benchmark-go/redisgraph-benchmark-go_darwin_amd64)

Here's an example on how to download and make the tool available on /usr/bin on darwin arch:
```
wget https://s3.amazonaws.com/benchmarks.redislabs/redisgraph/tools/redisgraph-benchmark-go/redisgraph-benchmark-go_darwin_amd64
chmod 755 redisgraph-benchmark-go_darwin_amd64
mv redisgraph-benchmark-go_darwin_amd64 /usr/bin/redisgraph-benchmark-go
```

## Usage

- Local benchmarks: `make benchmark`
- Remote benchmarks:  `make benchmark REMOTE=1`


## Included benchmarks

Each benchmark requires a benchmark definition yaml file to present on the current directory. The benchmark spec file is fully explained on the following link: https://github.com/RedisLabsModules/redisbench-admin/tree/master/docs

