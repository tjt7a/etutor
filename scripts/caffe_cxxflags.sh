#!/bin/sh
SCRIPTSPATH=`dirname $0`
[ -f $SCRIPTSPATH/../deps/caffe/BUILD/caffe/install/share/Caffe/CaffeConfig.cmake ] || exit 0
cat $SCRIPTSPATH/../deps/caffe/BUILD/caffe/install/share/Caffe/CaffeConfig.cmake | sed '/^.*set.*(.*Caffe_DEFINITIONS/!d' | sed -e 's/^.*Caffe_DEFINITIONS[ \t]*"\([^"]*\)".*$/\1/g' -e 's/;/ /g' 
