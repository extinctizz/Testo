#!/bin/bash
PATH=/bin:/usr/bin:/usr/sbin:/sbin
LADATE=$(date +"%y-%m-%d,%H:%M:%S")

#GENCORE_MAPS=""
#example:
#GENCORE_MAPS="603 631"

#This script is called each time the anti-crash triggers, with $1 == PID, and $2 == MapID

#generate backtrace
nice -n 20 gdb -batch -x /path/to/anticrash/backtrace.gdb /path/to/bin/worldserver $1 > /path/to/anticrash/log/backtrace.${LADATE}

#Do not generate coredump if last one is younger than 2hours

RESULT=$(find /path/to/anticrash/.lastcore -mmin +7200)
if [ "$RESULT" = "" ]; then
  exit 0
fi
touch /path/to/anticrash/.lastcore

#generate coredump
for M in $GENCORE_MAPS ; do
	if [ "$M" = "$2" ]; then
	nice -n 20 gcore -o /path/to/anticrash/coredump/core.${LADATE} $1
	fi
done
