#!/bin/bash

GDBARMPATH=/opt/fumanoids-arm/sat/bin/
GDBARM=arm-none-eabi-gdb

usageInfo ()
{
	echo "Use this function as follow"
	echo
	echo "\"JLinkGDBServer -if swd\" needs to be running"
	echo
	echo "\$ ./bin/flash.sh erolfP1.elf"
	echo "or"
	echo "\$ ./bin/flash.sh erolfP2.elf"
}


# Check if arm-none-eabi-gdb is in $PATH available
command -v arm-none-eabi-gdb > /dev/null 2>&1
if [ $? != 0 ];
then
	# Check if arm-none-eabi-gdb is GDBARMPATH available and executable
	if [ ! -f $GDBARMPATH$GDBARM ] ||
	   [ ! -x $GDBARMPATH$GDBARM ];
	then
		echo "arm-none-eabi-gdb konnte nicht gefunden werden oder ist nicht ausführbar."
		exit 1
	else
		GDBARM=$GDBARMPATH$GDBARM
	fi
fi

# Check if JLinkGDBServer is available
ps a | grep -v grep | grep -c "JLinkGDBServer -if swd" > /dev/null
if [ $? != 0 ];
then
	echo "JLinkGDBServer scheint nicht zu laufen"; exit 1;
fi

# Check if valid Parameter is given
if [ -z "$1" ];
then
	usageInfo
	exit 1
fi
if [ ! -f $1 ] || [ ! -r $1 ] || [ ! -s $1 ];
then
	echo "Datei $1 konnte nicht gelesen werden oder hat die Groeße 0 byte"
	exit 1
fi

# switching to the root folder of the project
cd "$(readlink -f "$(dirname "$0")/..")"

# Start flashing process
$GDBARM -batch -x bin/flashGDB $1
