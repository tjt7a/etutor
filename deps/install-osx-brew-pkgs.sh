#!/bin/bash

function brew_install() {
	brew list $1 >/dev/null 2>&1 && return 0
	echo "Installing $1"
	echo brew install $*
	brew install $*
}

# Homebrew installs are local to the current user
brew update
brew_install coreutils
brew_install autoconf
brew_install automake
brew_install libtool
brew_install cmake
# OpenCV
brew_install libdc1394
brew_install ffmpeg \
	--with-libvpx \
	--with-opus \
	--with-openjpeg \
	--with-openh264 \
	--with-faac \
	--with-fdk-aac \
	--with-x265 \
	--with-libvorbis \
	--with-libvidstab \
	--with-snappy \
	--with-xz \
	--with-tools

# gRPC
brew_install openssl
brew_install maven
brew_install gradle

# Berkely Caffe deps
# Don't install protobuf because we compile v3 from source
brew_install snappy 
brew_install leveldb
brew_install gflags 
brew_install glog 
brew_install szip 
brew_install lmdb
brew_install boost --build-from-source
brew_install boost-python --build-from-source

# OpenCV and Caffe
brew tap homebrew/science
#opencv --with-cuda  --with-vtk --with-contrib --with-quicktime
# LDFLAGS:  -L/usr/local/opt/opencv3/lib
# CPPFLAGS: -I/usr/local/opt/opencv3/include
# PKG_CONFIG_PATH: /usr/local/opt/opencv3/lib/pkgconfig
# If you need Python to find bindings for this keg-only formula, run:
# echo /usr/local/opt/opencv3/lib/python2.7/site-packages >> /usr/local/lib/python2.7/site-packages/opencv3.pth
# mkdir -p /Users/paul/.local/lib/python2.7/site-packages
# echo 'import site; site.addsitedir("/usr/local/lib/python2.7/site-packages")' >> /Users/paul/.local/lib/python2.7/site-packages/homebrew.pth
# brew_install opencv3 --c++11 --with-ffmpeg --with-gstreamer --with-java --with-libdc1394 --with-opengl
if ! grep '/usr/local/opt/opencv3/lib/python2.7/site-packages' /usr/local/lib/python2.7/site-packages/opencv3.pth 2>/dev/null; then
	echo /usr/local/opt/opencv3/lib/python2.7/site-packages >> /usr/local/lib/python2.7/site-packages/opencv3.pth
	mkdir -p ~/.local/lib/python2.7/site-packages
	echo 'import site; site.addsitedir("/usr/local/lib/python2.7/site-packages")' >> ~/.local/lib/python2.7/site-packages/homebrew.pth
fi
# need the homebrew science source for OpenCV and hdf5
brew_install hdf5
# TODO: if Nvidia GPU is installed then add --with-cuda option
brew_install opencv --c++11 --with-ffmpeg --with-gstreamer --with-java --with-libdc1394 --with-opengl
if python -c "import sys;print sys.version" | grep -i 'anaconda\|continuum' >/dev/null 2>/dev/null; then
	# Need to change some lines in opencv
	echo "WARNING: You are using Anaconda python!. See http://caffe.berkeleyvision.org/install_osx.html" 
	echo "--begin-snip-from-opencv-formula--"
	brew cat opencv | grep 'PYTHON_LIBRARY\|PYTHON_INCLUDE_DIR'
	echo "--end-snip-from-opencv-formula--"
fi
# pseudo terminal
brew tap homebrew/dupes
brew_install screen 

