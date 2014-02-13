#!/bin/bash

# Install the dependencies 

# determine linux distro
if [ $(which lsb_release 2>/dev/null) ]
then
  distro=$(lsb_release -d | awk {'print $2'} | tr '[:upper:]' '[:lower:]')
  if [ $distro == "linux" ]; then
    distro=$(lsb_release -d | awk {'print $3'} | tr '[:upper:]' '[:lower:]')
  fi
else
  # looking for release files - ugly - should work
  # http://linuxmafia.com/faq/Admin/release-files.html
  distro=$(ls /etc/ | egrep '[-_](release|version)$' | awk -F [-_] {'print $1'} \
           | tr '[:upper:]' '[:lower:]')
fi

case $distro in
  ubuntu|mint)
    packages="
	git \
	build-essential \
	libmpc2:i386 \
	libmpc-dev \
	python-yaml \
"

    echo "Installing all dependencies: $packages"

    sudo apt-get install $packages

    ;;

  arch)

    # Tested on Arch Linux Oct 2010
    
    echo
    echo "**WARNING** On x86_84 systems make sure to uncomment multilib in /etc/pacman.conf"
    echo

    packages="git build-essential python2-yaml"

    echo "Installing all dependencies: $packages"

    sudo pacman -S $packages

    ;;

  *)

    echo "Sorry, your OS is not supported yet"

    ;;
esac

#WE NOW USE THE PRECOMPILED BINARY - CALL ./bin/update-sdk.sh TO INSTALL !
#if [[ ! -d $HOME/sat ]]; then
#	echo "retrieving summon arm toolchain to tmp"
#	WORKING_DIR=`pwd`
#	mkdir -p /tmp/sat/summon-arm-toolchain
#	cd /tmp/sat/
#	git clone https://github.com/esden/summon-arm-toolchain.git
#	cd summon-arm-toolchain
#	chmod a+x summon-arm-toolchain
#	./summon-arm-toolchain && ( cd ..; rm -r summon-arm-toolchain )
#else
#	echo "Summon ARM toolchain seems to be already installed."
#	echo "To re-install, rm -r $HOME/sat"
#fi

