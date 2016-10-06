#!/usr/bin/env python

import os, sys, re, platform
import subprocess
import ConfigParser
import collections
from optparse import OptionParser


def die(msg=None):
    if msg is not None:
        print("Error: %s" % msg)
    sys.exit(1)


if __name__ == '__main__':
    usage = 'pygrep.py [-i|-m] <regex> [file]'
    parser = OptionParser(usage)
    parser.add_option('-i', '--ignore-case', action='store_true', dest='ignoreCase', help='case insensitive match')
    parser.add_option('-m', '--multiline', action='store_true', dest='multiline', help='multiline match')
    (options, args) = parser.parse_args()
    if args is None or len(args) == 0:
        die('missing regex')
    elif len(args) > 2:
        die('> 2 args')

    flags = 0;
    if options.ignoreCase:
        flags=re.IGNORECASE
    if options.multiline:
        flags=re.MULTILINE
    query = re.compile(args[0], flags=flags)
    if len(args) == 2:
        with open(args[1], 'r') as FIN:
            result = FIN.read()
    else:
        result = sys.stdin.read()

    srch = query.search(result)
    if srch is not None:
        print('<Match: %r, groups=%r>' % (srch.group(), srch.groups()))
