#! /bin/sh
# Upgrade CMake.
add-apt-repository -y ppa:george-edison55/cmake-3.x
apt-get -y update
apt-get install -y cmake
apt-get -y upgrade
exit 0
