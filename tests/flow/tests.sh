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

		[ARGVARS...] tests.sh [--help|help] [<module-so-path>]

		Argument variables:
		MODULE=path         Module .so path

		RLEC=0|1            General tests on RLEC

		REDIS_SERVER=path   Location of redis-server

		TEST=test           Run specific test (e.g. test.py:test_name)
		VALGRIND|VG=1       Run with Valgrind

		DOCKER_HOST         Address of Docker server (default: localhost)
		RLEC_PORT           Port of existing-env in RLEC container (default: 12000)

		RLTEST_ARGS=...     Extra RLTest arguments
		VERBOSE=1           Print commands
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
	export VG_OPTIONS="
		-q \
		--leak-check=full \
		--show-reachable=no \
		--track-origins=yes \
		--show-possibly-lost=no"

	# To generate supressions and/or log to file
	# --gen-suppressions=all --log-file=valgrind.log

	VALGRIND_SUPRESSIONS=$ROOT/src/valgrind.supp

	VALGRIND_ARGS+="\
		--no-output-catch \
		--use-valgrind \
		--vg-verbose \
		--vg-suppressions $VALGRIND_SUPRESSIONS"

}

#----------------------------------------------------------------------------------------------

run_tests() {
	local title="$1"
	if [[ -n $title ]]; then
		$READIES/bin/sep -0
		printf "Running $title:\n\n"
	fi

	config=$(mktemp "${TMPDIR:-/tmp}/rltest.XXXXXXX")
	rm -f $config
	if [[ $RLEC != 1 ]]; then
		cat << EOF > $config

--clear-logs
--oss-redis-path=$REDIS_SERVER
--module $MODULE
--module-args '$MODARGS'
$RLTEST_ARGS
$VALGRIND_ARGS

EOF
	else
		cat << EOF > $config

--clear-logs
$RLTEST_ARGS
$VALGRIND_ARGS

EOF
	fi

	cd $ROOT/tests/flow
	if [[ $VERBOSE == 1 ]]; then
		echo "RLTest configuration:"
		cat $config
	fi

	if [[ $PYHACK == 1 ]]; then
		viewdir=$(cd $HERE/.. && pwd)
		[[ -d $viewdir/RLTest ]] && PYTHONPATH=$viewdir/RLTest:$PYTHONPATH
		[[ -d $viewdir/redis-py ]] && PYTHONPATH=$viewdir/redis-py:$PYTHONPATH
		[[ -d $viewdir/redis-py-cluster ]] && PYTHONPATH=$viewdir/redis-py-cluster:$PYTHONPATH
		export PYTHONPATH
	fi

	[[ $RLEC == 1 ]] && export RLEC_CLUSTER=1

	if [[ $NOP != 1 ]]; then
		{ $OP python3 -m RLTest @$config; E=$?; } || true
	else
		$OP python3 -m RLTest @$config
		E=0
	fi

	[[ $KEEP != 1 ]] && rm -f $config
	return $E
}

#----------------------------------------------------------------------------------------------

[[ $1 == --help || $1 == help ]] && {
	help
	exit 0
}

OP=""
[[ $NOP == 1 ]] && OP=echo

#----------------------------------------------------------------------------------------------

RLEC=${RLEC:-0}

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

GDB=${GDB:-0}

[[ $VG == 1 ]] && VALGRIND=1
if [[ $VALGRIND == 1 ]]; then
	valgrind_config
fi

if [[ -n $TEST ]]; then
	RLTEST_ARGS+=" --test $TEST"
	export BB=${BB:-1}
fi

[[ $VERBOSE == 1 ]] && RLTEST_ARGS+=" -v"
[[ $GDB == 1 ]] && RLTEST_ARGS+=" -i --verbose"

#----------------------------------------------------------------------------------------------

cd $ROOT/tests/flow

[[ $RLEC != 1 ]] && setup_redis_server

NOTE=""
RLTEST_ARGS+=" --print-server-cmd"

E=0
if [[ $RLEC != 1 ]]; then
	{ (RLTEST_ARGS="${RLTEST_ARGS}" run_tests "general tests$NOTE"); (( E |= $? )); } || true
else
	{ (RLTEST_ARGS+=" --env existing-env --existing-env-addr $DOCKER_HOST:$RLEC_PORT" run_tests "tests on RLEC"); (( E |= $? )); } || true
fi

exit $E
