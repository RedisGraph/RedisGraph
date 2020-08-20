#!/bin/bash

# Collect name of each flow log that contains leaks.
if FLOW_FILES_WITH_LEAKS=$(grep -l "definitely lost: [1-9][0-9,]* bytes" flow/logs/*.valgrind.log); then
	echo "Memory leaks introduced in flow tests."
	# Print the full Valgrind output for each leaking file.
	for file in $FLOW_FILES_WITH_LEAKS; do
		cat "$file"
	done
	exit 1
fi

# Collect name of each TCK log that contains leaks.
if TCK_FILES_WITH_LEAKS=$(grep -l "definitely lost: [1-9][0-9,]* bytes" tck/logs/*.valgrind.log); then
	echo "Memory leaks introduced in TCK tests:"
	# Print the full Valgrind output for each leaking file.
	for file in $TCK_FILES_WITH_LEAKS; do
		cat "$file"
	done
	exit 1
fi

exit 0
