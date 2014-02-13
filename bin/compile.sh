#!/bin/bash

# the script assumes you are in the root folder of the project, make sure this is so
cd "$(readlink -f "$(dirname "$0")/..")"


###############################################################################

DEFAULT_PLATFROM=arm
PLATFORM=${1:-$DEFAULT_PLATFROM}

DEFAULT_CONFIG=debug
CONFIG=${2:-$DEFAULT_CONFIG}

DEFAULT_OPTION=""
OPTION=${3:-$DEFAULT_OPTION}

if [ "$OPTION" != "$DEFAULT_OPTION" ]
then
	OPTION="--$OPTION"
fi

# if verbose is not set, disable verbosity
if [[ -z ${VERBOSE+xxx} ]]; then
        VERBOSE=""
elif [[ ! -z $VERBOSE ]]; then
        VERBOSE="verbose=1"
fi


###############################################################################

# check that submodules are fetched
cat .gitmodules | grep path | while read -r path; do
	path=`echo $path | sed -e "s/^.*=//" | sed -e "s/ //"`
	if [[ ! -e $path/.git ]]; then
		echo
		echo "*** GIT submodule in $path does not exist, going to init/update now ***"
		echo "*** Note that this may require you to upload an SSH key to Redmine! ***"
		git submodule init
		git submodule update
	fi
done


###############################################################################

echo -e "Compiling erolf project for $PLATFORM\n"

rm -f Makefile
rm -f erolf.make

BRANCHNAME="`git symbolic-ref HEAD 2> /dev/null | cut -b 12-`"
LASTHASH="`git log --pretty=format:%h -1`"
DATE="`date +%Y%m%d-%H%M`"
GITUSERNAME="`git config user.name`"
USERINFO="$GITUSERNAME"
DIRTY=""
git diff-files --quiet || DIRTY=" (dirty)"
BUILDINFO="$BRANCHNAME-$LASTHASH$DIRTY build on $DATE by $USERINFO"
echo -e "#ifdef BUILDINFO\n#undef BUILDINFO\n#endif\n#define BUILDINFO \""$BUILDINFO"\"" > src/build_info.h

for component in vector p1 p2; do
	bin/premake4 --platform=$PLATFORM --component=$component $OPTION gmake
	make config=$CONFIG ${VERBOSE} all || break
done
