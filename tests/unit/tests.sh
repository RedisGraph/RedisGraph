#!/bin/bash

PROGNAME="${BASH_SOURCE[0]}"
HERE="$(cd "$(dirname "$PROGNAME")" &>/dev/null && pwd)"
ROOT=$(cd $HERE/../.. && pwd)
READIES=$ROOT/deps/readies
. $READIES/shibumi/defs

cd $HERE

#----------------------------------------------------------------------------------------------

help() {
	cat <<-'END'
		Run unit tests

		[ARGVARS...] unit-tests [--help|help]

		Argument variables:
		BINROOT=path   Path to repo binary root dir
		TEST=name      Operate in single-test mode

        SAN=addr|mem   Run with sanitizer
		TEST_LEAK=1    Run test that leaks (for sanitizer diagnostics)

		GDB=1          Enable interactive gdb debugging (in single-test mode)
		CLANG=1        Implies use of lldb debugger
		VERBOSE=1      Print commands and Redis output
		NOP=1          Dry run
		HELP=1         Show help


	END
	exit 0
}

#----------------------------------------------------------------------------------------------

sanitizer_defs() {
	if [[ -n $SAN ]]; then
		ASAN_LOG=${LOGS_DIR}/${TEST_NAME}.asan.log
		export ASAN_OPTIONS="detect_odr_violation=0:halt_on_error=0:log_path=${ASAN_LOG}"
		export LSAN_OPTIONS="suppressions=$ROOT/tests/memcheck/asan.supp"
	fi
}

#----------------------------------------------------------------------------------------------

sanitizer_summary() {
	if grep -l "leaked in" ${LOGS_DIR}/*.asan.log* &> /dev/null; then
		echo
		echo "${LIGHTRED}Sanitizer: leaks detected:${RED}"
		grep -l "leaked in" ${LOGS_DIR}/*.asan.log*
		echo "${NOCOLOR}"
		E=1
	fi
}

#----------------------------------------------------------------------------------------------

[[ $1 == --help || $1 == help || $HELP == 1 ]] && { help; exit 0; }

OP=
[[ $NOP == 1 ]] && OP=echo

TEST_LEAK=${TEST_LEAK:-0}

LOGS_DIR=$ROOT/tests/unit/logs

if [[ $CLANG == 1 ]]; then
	GDB_CMD="lldb -o run --"
else
	GDB_CMD="gdb -ex r --args"
fi

#----------------------------------------------------------------------------------------------

if [[ $CLEAR_LOGS != 0 ]]; then
	rm -rf $LOGS_DIR
fi
mkdir -p $LOGS_DIR

E=0

$READIES/bin/sep
echo "# Running unit tests"
TESTS_DIR="$(cd $BINROOT/src/tests/unit; pwd)"
cd $ROOT/tests/unit
if [[ -z $TEST ]]; then
	if [[ $NOP != 1 ]]; then
		for test in $(find $TESTS_DIR -name "test_*" -type f -executable -print); do
			test_name="$(basename $test)"
			if [[ $TEST_LEAK == 1 || $test_name != test_leak ]]; then
				TEST_NAME="$test_name" sanitizer_defs
				{ $test; (( E |= $? )); } || true
			fi
		done
	else
		find $TESTS_DIR -name "test_*" -type f -executable -print
	fi
else
	$OP $GDB_CMD $TESTS_DIR/$TEST
fi

if [[ -n $SAN ]]; then
	sanitizer_summary
fi

exit $E
