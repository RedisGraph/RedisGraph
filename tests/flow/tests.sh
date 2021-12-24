#!/bin/bash

# [[ $VERBOSE == 1 ]] && set -x
[[ $IGNERR == 1 ]] || set -e

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
ROOT=$(cd $HERE/../.. && pwd)
READIES=$ROOT/deps/readies 
. $READIES/shibumi/defs

VALGRIND_REDIS_VER=6.2

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

		RLTEST_ARGS=...     Extra RLTest arguments
		V|VERBOSE=1         Print commands
		IGNERR=1            Do not abort on error

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
	export VG_OPTIONS="
		-q \
		--leak-check=full \
		--show-reachable=no \
		--track-origins=yes \
		--show-possibly-lost=no"

	# To generate supressions and/or log to file
	# --gen-suppressions=all --log-file=valgrind.log

	[[ $VG_LEAKS == 0 ]] && VALGRIND_ARGS+=" --vg-no-leakcheck"

	VALGRIND_SUPRESSIONS=$ROOT/src/valgrind.sup

	VALGRIND_ARGS+="\
		--no-output-catch \
		--use-valgrind \
		--vg-verbose \
		--vg-suppressions $VALGRIND_SUPRESSIONS"

	export RS_GLOBAL_DTORS=1
	export VALGRIND=1
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
				--clear-logs
				--oss-redis-path=$REDIS_SERVER
				--module $MODULE
				--module-args '$MODARGS'
				$RLTEST_ARGS
				$RLTEST_VG_ARGS

				EOF
		else
			cat <<-EOF > $rltest_config
				--clear-logs
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

E=0
[[ $GEN == 1 ]]  && { (run_tests "general tests"); (( E |= $? )); } || true
[[ $AOF == 1 ]]  && { (RLTEST_ARGS="${RLTEST_ARGS} --use-aof --test test_persistency" run_tests "tests with AOF"); (( E |= $? )); } || true
[[ $TCK == 1 ]]  && { (cd ../tck; run_tests "TCK tests"); (( E |= $? )); } || true

[[ $RLEC == 1 ]] && { (RLTEST_ARGS="${RLTEST_ARGS} --env existing-env --existing-env-addr $DOCKER_HOST:$RLEC_PORT" run_tests "tests on RLEC"); (( E |= $? )); } || true

exit $E
