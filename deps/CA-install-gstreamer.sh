#! /bin/sh
add-apt-repository -y ppa:gstreamer-developers/ppa
apt-get -y update
apt-get install -y libjansson-dev
exit 0
