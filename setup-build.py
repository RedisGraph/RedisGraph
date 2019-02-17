#!/usr/bin/env python

import os
import platform

dist = platform.dist()[0]
common_tools = "cmake flex bison"
if dist == 'Ubuntu':
	os.system('apt-get install -y build-essential {}'.format(common_tools))
elif dist == 'fedora' or dist == 'centos':
	os.system('dnf groupinstall -y "Development Tools"')
	os.system('dnf install -y {}'.format(common_tools))
else:
	print("please install gcc, automake, flex, bison, and cmake\n")
	exit(1)

exit(0)
