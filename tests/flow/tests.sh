#!/bin/bash

# [[ $VERBOSE == 1 ]] && set -x

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
ROOT=$(cd $HERE/../.. && pwd)
READIES=$ROOT/deps/readies 
. $READIES/shibumi/defs

VALGRIND_REDIS_VER=6

#----------------------------------------------------------------------------------------------

help() {
	cat <<-END
		Run flow tests.

		[ARGVARS...] tests.sh [--help|help]

		Argument variables:
		MODULE=path         Module .so path

		TEST=test           Run specific test (e.g. test.py:test_name)

		GEN=1               General tests on standalone Redis (default)
		AOF=1               AOF persistency tests on standalone Redis
		TCK=1               Cypher Technology Compatibility Kit tests
		RLEC=0|1            General tests on RLEC

		REDIS_SERVER=path   Location of redis-server

		EXT|EXISTING_ENV=1  Run the tests on existing env
		EXT_HOST=addr:port  Address of Redis server (default: 127.0.0.1:6379)

		VALGRIND|VG=1       Run with Valgrind
		VG_LEAKS=1          Look for memory leaks
		VG_ACCESS=1         Look for memory access errors

		DOCKER_HOST         Address of Docker server (default: localhost)
		RLEC_PORT           Port of existing-env in RLEC container (default: 12000)

		PLATFORM_MODE=1     Implies NOFAIL & COLLECT_LOGS into STATFILE
		CLEAR_LOGS=0        Do not remove logs prior to running tests
		COLLECT_LOGS=1      Collect logs into .tar file
		NOFAIL=1            Do not fail on errors (always exit with 0)
		STATFILE=file       Write test status (0|1) into `file`

		RLTEST_ARGS=...     Extra RLTest arguments
		V|VERBOSE=1         Print commands

	END
}

#----------------------------------------------------------------------------------------------

setup_redis_server() {
	if [[ $VALGRIND == 1 ]]; then
		REDIS_SERVER=${REDIS_SERVER:-redis-server-vg}
		if ! is_command $REDIS_SERVER; then
			echo Building Redis for Valgrind ...
			$READIES/bin/getredis -v $VALGRIND_REDIS_VER --valgrind --suffix vg
		fi
	else
		REDIS_SERVER=${REDIS_SERVER:-redis-server}
	fi

	if ! is_command $REDIS_SERVER; then
		echo "Cannot find $REDIS_SERVER. Aborting."
		exit 1
	fi
}

#----------------------------------------------------------------------------------------------

valgrind_config() {
	# RLTest reads this
	VG_OPTIONS="
		-q \
		--show-reachable=no \
		--track-origins=yes \
		--show-possibly-lost=no"

	# To generate supressions and/or log to file
	# --gen-suppressions=all --log-file=valgrind.log

	if [[ $VG_LEAKS == 0 ]]; then
		RLTEST_VG_ARGS+=" --vg-no-leakcheck"
		VG_OPTIONS+=" --no-output-catch"
	else
		VG_OPTIONS+=" --leak-check=full"
	fi

	VALGRIND_SUPRESSIONS=$ROOT/tests/valgrind/valgrind.supp

	RLTEST_VG_ARGS+="\
		--use-valgrind \
		--vg-verbose \
		--vg-no-fail-on-errors \
		--vg-suppressions $VALGRIND_SUPRESSIONS"

	export RS_GLOBAL_DTORS=1
	export VALGRIND=1
	export VG_OPTIONS
	export RLTEST_VG_ARGS
}

#----------------------------------------------------------------------------------------------

run_tests() {
	local title="$1"
	if [[ -n $title ]]; then
		$READIES/bin/sep -0
		printf "Running $title:\n\n"
	fi

	if [[ $EXISTING_ENV != 1 ]]; then
		rltest_config=$(mktemp "${TMPDIR:-/tmp}/rltest.XXXXXXX")
		if [[ $RLEC != 1 ]]; then
			cat <<-EOF > $rltest_config
				# --clear-logs
				--oss-redis-path=$REDIS_SERVER
				--module $MODULE
				--module-args '$MODARGS'
				$RLTEST_ARGS
				$RLTEST_VG_ARGS

				EOF
		else
			cat <<-EOF > $rltest_config
				# --clear-logs
				$RLTEST_ARGS
				$RLTEST_VG_ARGS

				EOF
		fi
	else # existing env
		rltest_config=$(mktemp "${TMPDIR:-/tmp}/xredis_rltest.XXXXXXX")
		cat <<-EOF > $rltest_config
			--env existing-env
			--existing-env-addr $EXT_HOST
			$RLTEST_ARGS

			EOF
	fi

	if [[ $VERBOSE == 1 ]]; then
		echo "RLTest configuration:"
		cat $rltest_config
	fi

	[[ $RLEC == 1 ]] && export RLEC_CLUSTER=1
	
	local E=0
	if [[ $NOP != 1 ]]; then
		{ $OP python3 -m RLTest @$rltest_config; (( E |= $? )); } || true
	else
		$OP python3 -m RLTest @$rltest_config
	fi

	[[ $KEEP != 1 ]] && rm -f $rltest_config

	return $E
}

#----------------------------------------------------------------------------------------------

[[ $1 == --help || $1 == help ]] && {
	help
	exit 0
}

OP=""
[[ $NOP == 1 ]] && OP=echo

[[ $V == 1 ]] && VERBOSE=1

#----------------------------------------------------------------------------------------------

if [[ $PLATFORM_MODE == 1 ]]; then
	CLEAR_LOGS=0
	COLLECT_LOGS=1
	NOFAIL=1
	STATFILE=$ROOT/bin/artifacts/tests/status
fi

#----------------------------------------------------------------------------------------------

RLEC=${RLEC:-0}

[[ $EXT == 1 ]] && EXISTING_ENV=1
EXT_HOST=${EXT_HOST:-127.0.0.1:6379}

if [[ $RLEC != 1 ]]; then
	GEN=${GEN:-1}
	AOF=${AOF:-1}
	TCK=${TCK:-0}
else
	GEN=0
	AOF=0
	TCK=0
fi

#----------------------------------------------------------------------------------------------

GDB=${GDB:-0}
[[ $GDB == 1 ]] && RLTEST_ARGS+=" -i --verbose"

VG_LEAKS=${VG_LEAKS:-1}
VG_ACCESS=${VG_ACCESS:-1}

[[ $VG == 1 ]] && VALGRIND=1
[[ $VALGRIND == 1 ]] && valgrind_config

#----------------------------------------------------------------------------------------------

DOCKER_HOST=${DOCKER_HOST:-localhost}
RLEC_PORT=${RLEC_PORT:-12000}

#----------------------------------------------------------------------------------------------

if [[ $RLEC != 1 ]]; then
	MODULE=${MODULE:-$1}
	[[ -z $MODULE || ! -f $MODULE ]] && {
		echo "Module not found at ${MODULE}. Aborting."
		exit 1
	}
fi

if [[ -n $TEST ]]; then
	RLTEST_ARGS+=" --test $TEST"
	export BB=${BB:-1}
fi

[[ $VERBOSE == 1 ]] && RLTEST_ARGS+=" -v"

[[ $RLEC != 1 ]] && setup_redis_server

#----------------------------------------------------------------------------------------------

cd $ROOT/tests/flow
if [[ $CLEAR_LOGS != 0 ]]; then
	rm -rf logs ../tck/logs
fi

E=0
[[ $GEN == 1 ]]  && { (run_tests "general tests"); (( E |= $? )); } || true
[[ $AOF == 1 ]]  && { (RLTEST_ARGS="${RLTEST_ARGS} --use-aof --test test_persistency" run_tests "tests with AOF"); (( E |= $? )); } || true
[[ $TCK == 1 ]]  && { (cd ../tck; run_tests "TCK tests"); (( E |= $? )); } || true

[[ $RLEC == 1 ]] && { (RLTEST_ARGS="${RLTEST_ARGS} --env existing-env --existing-env-addr $DOCKER_HOST:$RLEC_PORT" run_tests "tests on RLEC"); (( E |= $? )); } || true

#---------------------------------------------------------------------------------------------- 

if [[ $COLLECT_LOGS == 1 ]]; then
	ARCH=`$READIES/bin/platform --arch`
	OSNICK=`$READIES/bin/platform --osnick`
	cd $ROOT
	mkdir -p bin/artifacts/tests
	find tests/flow/logs -name "*.log" | tar -czf bin/artifacts/tests/tests-flow-logs-${ARCH}-${OSNICK}.tgz -T -
	find tests/tck/logs -name "*.log" | tar -czf bin/artifacts/tests/tests-tck-logs-${ARCH}-${OSNICK}.tgz -T -
fi

if [[ -n $STATFILE ]]; then
	mkdir -p $(dirname $STATFILE)
	if [[ -f $STATFILE ]]; then
		(( E |= `cat $STATFILE || echo 1` ))
	fi
	echo $E > $STATFILE
fi

if [[ $NOFAIL == 1 ]]; then
	exit 0
fi
exit $E
