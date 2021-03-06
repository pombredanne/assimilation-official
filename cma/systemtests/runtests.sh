#!/bin/sh
# vim: smartindent tabstop=4 shiftwidth=4 expandtab number
#
# This file is part of the Assimilation Project.
#
# Author: Alan Robertson <alanr@unix.sh>
# Copyright (C) 2014 - Assimilation Systems Limited
#
# Free support is available from the Assimilation Project community - http://assimproj.org
# Paid support is available from Assimilation Systems Limited - http://assimilationsystems.com
#
# The Assimilation software is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Assimilation software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Assimilation Project software.  If not, see http://www.gnu.org/licenses/
#
#   Script to run system tests and save results and syslog into a unique directory.
#
if [ $# -eq 0 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    python assimtest.py
    exit
fi
dirname=$(date --iso-8601=seconds)
echo $name
mkdir $dirname
LOGMSG="STARTING ASSIMILATION TESTS in $dirname with ARGS: $@"
LOGNAME="$dirname/testlog.txt"
SYSLOGTAIL="$dirname/syslog"
echo $LOGMSG > $LOGNAME
echo '# vim: syntax=messages' > $SYSLOGTAIL
logger -s "$LOGMSG" 2>$LOGNAME
SYSLOG=`echo $@ | sed 's/.*-l  *\([^ ]*\).*/\1/'`
: ${SYSLOG="/var/log/syslog"}
tail -1f $SYSLOG >> $SYSLOGTAIL &
syslogpid=$!
time python assimtest.py "$@" >>$LOGNAME 2>&1 &
testpid=$!
tail -f $LOGNAME &
tailpid=$!
wait $testpid
rc=$?
kill $tailpid
kill $syslogpid
echo "Tests complete with rc $rc"
exit $rc
