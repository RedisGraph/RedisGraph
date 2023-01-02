#!/bin/bash

PROGNAME="${BASH_SOURCE[0]}"
HERE="$(cd "$(dirname "$PROGNAME")" &>/dev/null && pwd)"
ROOT=$(cd $HERE/.. && pwd)
export READIES=$ROOT/deps/readies
. $READIES/shibumi/defs

cd $HERE

#----------------------------------------------------------------------------------------------

valgrind_summary() {
	local logdir="$ROOT/tests/$DIR/logs"
	local leaks_head=0
	for file in $(ls $logdir/*.valgrind.log 2>/dev/null); do
		# If the last "definitely lost: " line of a logfile has a nonzero value, print the file name
		if tac "$file" | grep -a -m 1 "definitely lost: " | grep "definitely lost: [1-9][0-9,]* bytes" &> /dev/null; then
			if [[ $leaks_head == 0 ]]; then
				echo
				echo "${LIGHTRED}### Leaks:${RED}"
				leaks_head=1
			fi
			echo "$file"
			E=1
		fi
	done
	echo -n "${NOCOLOR}"
	if grep -l "Invalid read" $logdir/*.valgrind.log &> /dev/null; then
		echo
		echo "${LIGHTRED}### Invalid reads:${RED}"
		grep -l "Invalid read" $logdir/*.valgrind.log
		echo -n "${NOCOLOR}"
		E=1
	fi
	if grep -l "Invalid write" $logdir/*.valgrind.log &> /dev/null; then
		echo
		echo "${LIGHTRED}### Invalid writes:${RED}"
		grep -l "Invalid write" $logdir/*.valgrind.log
		echo -n "${NOCOLOR}"
		E=1
	fi
}

#----------------------------------------------------------------------------------------------

sanitizer_summary() {
	local logdir="$ROOT/tests/$DIR/logs"
	if grep -l "leaked in" $logdir/*.asan.log* &> /dev/null; then
		echo
		echo "${LIGHTRED}Sanitizer: leaks detected:${RED}"
		grep -l "leaked in" $logdir/*.asan.log*
		echo "${NOCOLOR}"
		E=1
	fi
	if grep -l "dynamic-stack-buffer-overflow" $logdir/*.asan.log* &> /dev/null; then
		echo
		echo "${LIGHTRED}Sanitizer: buffer overflow detected:${RED}"
		grep -l "dynamic-stack-buffer-overflow" $logdir/*.asan.log*
		echo "${NOCOLOR}"
		E=1
	fi
	if grep -l "stack-use-after-scope" $logdir/*.asan.log* &> /dev/null; then
		echo
		echo "${LIGHTRED}Sanitizer: stack use after scope detected:${RED}"
		grep -l "stack-use-after-scope" $logdir/*.asan.log*
		echo "${NOCOLOR}"
		E=1
	fi
}

#----------------------------------------------------------------------------------------------

E=0

DIRS=
if [[ $UNIT == 1 ]]; then
	DIRS+=" unit"
fi
if [[ $FLOW == 1 ]]; then
	DIRS+=" flow"
	if [[ $TCK == 1 ]]; then
		DIRS+=" tck"
	fi
fi

if [[ $VG == 1 ]]; then
	for dir in $DIRS; do
		DIR="$dir" valgrind_summary
	done
elif [[ -n $SAN ]]; then
	for dir in $DIRS; do
		DIR="$dir" sanitizer_summary
	done
fi

exit $E
