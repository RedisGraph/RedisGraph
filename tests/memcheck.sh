#!/bin/bash

rc=0
# For each Valgrind log in the flow and TCK tests,
# print contents if memory has definitely been lost
for testdir in flow tck; do
	for file in $(ls $testdir/logs/*.valgrind.log); do
		# If the last "definitely lost: " line of a logfile
		# has a nonzero value, print the file contents.
		if tac "$file" | grep -a -m 1 "definitely lost: " | grep "definitely lost: [1-9][0-9,]* bytes"; then
			cat "$file"
			rc=1
		fi
	done
done

exit $rc
