#!/bin/sh

aclocal
if which libtoolize 2>/dev/null 1>/dev/null ; then
	libtoolize --automake --force
elif which glibtoolize 2>/dev/null 1>/dev/null ; then
	glibtoolize --automake --force
else
	echo "Error could not find libtoolize or glibtoolize"
	exit 1
fi
autoheader
autoconf -f
# Why would I give a damn about supporting AM 1.6
# no-portability - bring it on
automake -a -c -Wno-portability
