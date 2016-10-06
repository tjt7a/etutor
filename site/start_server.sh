#!/bin/bash

die () {
	echo "Error: $1"
	exit 1
}

[ -f run/server.pid ] && kill -0 `cat run/server.pid` &>/dev/null && die "stop server first" 
mkdir -p run
source `pwd`/../../deps/vpython/ENV/vpython/bin/activate || die "cannot start virtual python environment"
python app.py $* &
PID="$!"
echo "$PID" > run/server.pid
#
#wait $PID
while kill -0 $PID &> /dev/null; do sleep 1; done
rm -f run/server.pid &>/dev/null
