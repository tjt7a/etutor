#!/bin/sh
uuidgen | sed 's/-/_/g' | tr 'a-z' 'A-Z'
