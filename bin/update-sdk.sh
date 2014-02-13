#!/bin/bash

# 1. get current version
# 2. get version from SVN
# 3. if they differ, download version from svn

mypath=`pwd`

SATHOME=/opt/fumanoids-arm/

SVNROOT="https://maserati.mi.fu-berlin.de/svn/binaries/summon-arm-toolchain"
INSTALL_COMPILER=0

if [[ ! -e $SATHOME ]]; then
	echo "Creating $SATHOME"
	sudo mkdir "$SATHOME"
	sudo chown -R $USER "$SATHOME"
fi

# get version numbers from svn
compiler_file=`svn ls $SVNROOT/ | grep summon-arm-toolchain | tail -1` # summon-arm-toolchain-20121113.tgz
compiler=`echo $compiler_file | sed -e 's/^[^0-9]*\([0-9\.\-]*\).*$/\1/g'`

if [[ -z $compiler_file ]]; then
	echo "Could not find a compiler file."
	exit
fi
# get current version numbers of local files
# and mark compiler/SDK for installation if necessary
if [ ! -e $SATHOME/.compiler.version ] ; then
	echo "You do not seem to have the compiler installed, or you installed it manually. I am going to replace it."

	INSTALL_COMPILER=1
fi

# install Compiler
if [[ $INSTALL_COMPILER -eq 1 ]]; then
	echo "Installing compiler $compiler_file"
	cd /tmp
	if [ ! -e "$compiler_file" ]; then
		echo "  Downloading compiler $compiler_file, please wait"
		svn export "$SVNROOT/$compiler_file" || exit
	fi
	rm -rf $SATHOME/sat || exit
	cd $SATHOME/
	tar xvzf "/tmp/$compiler_file" > /dev/null || exit
	echo "$compiler" > .compiler.version
	cd $mypath
fi

cd $mypath
echo "Update FINISHED."

