#!/bin/bash

if FLOW_LEAKS=$(grep "definitely lost: [1-9][0-9]* bytes" flow/logs/*.valgrind.log); then
	echo "Memory leaks introduced in flow tests:"
	echo "$FLOW_LEAKS"
	exit 1
fi

if TCK_LEAKS=$(grep "definitely lost: [1-9][0-9,]* bytes" tck/logs/*.valgrind.log); then
	echo "Memory leaks introduced in TCK tests:"
	echo "$TCK_LEAKS"
	exit 1
fi

exit 0