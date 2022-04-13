#!/bin/bash

PROGNAME="${BASH_SOURCE[0]}"
HERE="$(cd "$(dirname "$PROGNAME")" &>/dev/null && pwd)"
ROOT=$(cd $HERE/.. && pwd)
export READIES=$ROOT/deps/readies
. $READIES/shibumi/defs

cd $HERE

rc=0
# For each Valgrind log in the flow and TCK tests,
# print contents if memory has definitely been lost
for testdir in flow tck; do
	for file in $(ls $testdir/logs/*.valgrind.log 2>/dev/null); do
		# If the last "definitely lost: " line of a logfile
		# has a nonzero value, print the file contents.
		if tac "$file" | grep -a -m 1 "definitely lost: " | grep "definitely lost: [1-9][0-9,]* bytes"; then
			cat "$file"
			rc=1
		fi
	done
	if grep -l "Invalid read" $testdir/logs/*.valgrind.log &> /dev/null; then
		echo
		echo "### Invalid reads:"
		grep -l "Invalid read" $testdir/logs/*.valgrind.log
	fi
	if grep -l "Invalid write" $testdir/logs/*.valgrind.log &> /dev/null; then
		echo
		echo "### Invalid writes:"
		grep -l "Invalid write" $testdir/logs/*.valgrind.log
	fi
done

exit $rc
