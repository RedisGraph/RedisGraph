#!/bin/bash

PROGNAME="${BASH_SOURCE[0]}"
HERE="$(cd "$(dirname "$PROGNAME")" &>/dev/null && pwd)"
ROOT=$(cd $HERE/.. && pwd)
export READIES=$ROOT/deps/readies
. $READIES/shibumi/defs

cd $ROOT

export PYTHONWARNINGS=ignore

#----------------------------------------------------------------------------------------------

if [[ $1 == --help || $1 == help ]]; then
	cat <<-END
		[ARGVARS...] pack.sh [--help|help]
		
		Argument variables:
		VERBOSE=1           Print commands
		IGNERR=1            Do not abort on error

		RAMP=1              Build RAMP file
		DEPS=1              Build dependencies file

		PACKAGE_NAME=name   Package stem name
		VARIANT=name        Build variant (empty for standard packages)
		BRANCH=name         Branch name for snapshot packages
		GITSHA=1            Append Git SHA to shapshot package names

		BINDIR=dir          Directory in which packages are created
		INSTALL_DIR=dir     Directory in which artifacts are found

	END
	exit 0
fi

#----------------------------------------------------------------------------------------------

[[ $IGNERR == 1 ]] || set -e
[[ $V == 1 || $VERBOSE == 1 ]] && set -x

RAMP=${RAMP:-1}
DEPS=${DEPS:-1}

[[ -z $INSTALL_DIR ]] && INSTALL_DIR=src
INSTALL_DIR=$(cd $INSTALL_DIR && pwd)

[[ -z $BINDIR ]] && BINDIR=bin/artifacts
mkdir -p $BINDIR
BINDIR=$(cd $BINDIR && pwd)

export ARCH=$($READIES/bin/platform --arch)
[[ $ARCH == x64 ]] && ARCH=x86_64

export OS=$($READIES/bin/platform --os)
[[ $OS == linux ]] && OS=Linux

export OSNICK=$($READIES/bin/platform --osnick)
[[ $OSNICK == trusty ]]  && OSNICK=ubuntu14.04
[[ $OSNICK == xenial ]]  && OSNICK=ubuntu16.04
[[ $OSNICK == bionic ]]  && OSNICK=ubuntu18.04
[[ $OSNICK == focal ]]   && OSNICK=ubuntu20.04
[[ $OSNICK == centos7 ]] && OSNICK=rhel7
[[ $OSNICK == centos8 ]] && OSNICK=rhel8
[[ $OSNICK == ol8 ]]     && OSNICK=rhel8

export PRODUCT=redisgraph
export PRODUCT_LIB=$PRODUCT.so
export DEPNAMES=""

export PACKAGE_NAME=${PACKAGE_NAME:-${PRODUCT}}

RAMP_PROG="python3 -m RAMP.ramp"

#----------------------------------------------------------------------------------------------

pack_ramp() {
	cd $ROOT

	local platform="$OS-$OSNICK-$ARCH"
	local stem=${PACKAGE_NAME}.${platform}

	local verspec=${SEMVER}${_VARIANT}
	
	local fq_package=$stem.${verspec}.zip

	[[ ! -d $BINDIR ]] && mkdir -p $BINDIR

	local packfile="$BINDIR/$fq_package"
	local product_so="$INSTALL_DIR/$PRODUCT.so"

	local xtx_vars=""
	local dep_fname=${PACKAGE_NAME}.${platform}.${verspec}.tgz

	if [[ -z $RAMP_YAML ]]; then
		RAMP_YAML=$ROOT/ramp.yml
	fi

	python3 $READIES/bin/xtx \
		$xtx_vars \
		-e NUMVER -e SEMVER \
		$RAMP_YAML > /tmp/ramp.yml
	rm -f /tmp/ramp.fname $packfile
	$RAMP_PROG pack -m /tmp/ramp.yml --packname-file /tmp/ramp.fname --verbose --debug \
		-o $packfile $product_so >/tmp/ramp.err 2>&1 || true
	if [[ ! -e $packfile ]]; then
		eprint "Error generating RAMP file:"
		>&2 cat /tmp/ramp.err
		exit 1
	fi

	mkdir -p $BINDIR/snapshots
	cd $BINDIR/snapshots
	if [[ ! -z $BRANCH ]]; then
		local snap_package=$stem.${BRANCH}${_VARIANT}.zip
		ln -sf ../$fq_package $snap_package
	fi

	cd $ROOT
}

#----------------------------------------------------------------------------------------------

pack_deps() {
	local dep="$1"

	local platform="$OS-$OSNICK-$ARCH"
	local verspec=${SEMVER}${_VARIANT}
	local stem=${PACKAGE_NAME}-${dep}.${platform}

	local artdir=$BINDIR
	local depdir=$(cat $artdir/$dep.dir)

	local fq_dep=$stem.${verspec}.tgz
	local tar_path=$artdir/$fq_dep
	local dep_prefix_dir=$(cat $artdir/$dep.prefix)
	
	{ cd $depdir ;\
	  cat $artdir/$dep.files | \
	  xargs tar -c --sort=name --owner=root:0 --group=root:0 --mtime='UTC 1970-01-01' \
		--transform "s,^,$dep_prefix_dir," 2>> /tmp/pack.err | \
	  gzip -n - > $tar_path ; E=$?; } || true
	rm -f $artdir/$dep.prefix $artdir/$dep.files $artdir/$dep.dir

	cd $ROOT
	if [[ $E != 0 ]]; then
		eprint "Error creating $tar_path:"
		cat /tmp/pack.err >&2
		exit 1
	fi
	sha256sum $tar_path | awk '{print $1}' > $tar_path.sha256

	mkdir -p $BINDIR/snapshots
	cd $BINDIR/snapshots
	if [[ ! -z $BRANCH ]]; then
		local snap_dep=$stem.${BRANCH}${_VARIANT}.tgz
		ln -sf ../$fq_dep $snap_dep
		ln -sf ../$fq_dep.sha256 $snap_dep.sha256
	fi
	
	cd $ROOT
}

#----------------------------------------------------------------------------------------------

prepare_symbols_dep() {
	echo "Preparing debug symbols dependencies ..."
	echo $INSTALL_DIR > $BINDIR/debug.dir
	echo $PRODUCT.so.debug > $BINDIR/debug.files
	echo "" > $BINDIR/debug.prefix
	pack_deps debug
	echo "Done."
}

#----------------------------------------------------------------------------------------------

export NUMVER=$(NUMERIC=1 $ROOT/sbin/getver)
export SEMVER=$($ROOT/sbin/getver)

_VARIANT=
if [[ -n $VARIANT ]]; then
	_VARIANT=-${VARIANT}
fi

[[ -z $BRANCH ]] && BRANCH=${CIRCLE_BRANCH:-`git rev-parse --abbrev-ref HEAD`}
BRANCH=${BRANCH//[^A-Za-z0-9._-]/_}
if [[ $GITSHA == 1 ]]; then
	GIT_COMMIT=$(git describe --always --abbrev=7 --dirty="+" 2>/dev/null || git rev-parse --short HEAD)
	BRANCH="${BRANCH}-${GIT_COMMIT}"
fi
export BRANCH

if [[ $DEPS == 1 ]]; then
	echo "Building dependencies ..."

	prepare_symbols_dep

	for dep in $DEPNAMES; do
			echo "$dep ..."
			pack_deps $dep
	done
fi

if [[ $RAMP == 1 ]]; then
	if ! command -v redis-server > /dev/null; then
		eprint "$0: Cannot find redis-server. Aborting."
		exit 1
	fi

	echo "Building RAMP files ..."
	pack_ramp
	echo "Done."
fi

if [[ $VERBOSE == 1 ]]; then
	echo "Artifacts:"
	du -ah --apparent-size $BINDIR
fi

exit 0
