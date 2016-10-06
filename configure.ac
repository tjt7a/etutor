dnl Autotool for SubjectLens

AC_PREREQ(2.59)
AC_INIT([lucida], [0.1.0])
AC_CANONICAL_SYSTEM
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wno-portability])

dnl Extra target for starting servers
AM_EXTRA_RECURSIVE_TARGETS([start_server stop_server])

dnl ---------------------------------------------------------------------------
dnl Determine platform and architecture 

AC_MSG_CHECKING([platform version])
AS_IF(
	[python -mplatform | grep -qi ubuntu],
	[PLATFORM_NAME="Ubuntu";PLATFORM_TYPE="deb";],
	[AS_IF(
		[python -mplatform | grep -qi debian],
		[PLATFORM_NAME="Debian";PLATFORM_TYPE="deb";],
		[AS_IF(
			[python -mplatform | grep -qi darwin],
			[PLATFORM_NAME="Darwin";PLATFORM_TYPE="osx";],
			[AS_IF(
				[python -mplatform | grep -qi 'redhat\|rhel'],
				[PLATFORM_NAME="RHEL";PLATFORM_TYPE="rpm";],
				[AS_IF(
					[python -mplatform | grep -qi "centos"],
					[PLATFORM_NAME="CentOS";PLATFORM_TYPE="rpm";],
					[PLATFORM_NAME="Unknown";PLATFORM_TYPE="rpm";]
				)]
			)]
		)]
	)]
)

PLATFORM_MAJOR_VERSION=[$(python -mplatform | sed 's/^.*with[^0-9]*\([0-9]\+\)\([^0-9].*$\|$\)/\1/g')]
AC_MSG_RESULT([$PLATFORM_NAME $PLATFORM_MAJOR_VERSION])

AM_CONDITIONAL(PLATFORM_UBUNTU, [test "x$PLATFORM_NAME" = "xUbuntu"])
AM_CONDITIONAL(PLATFORM_DEBIAN, [test "x$PLATFORM_NAME" = "xDebian"])
AM_CONDITIONAL(PLATFORM_DARWIN, [test "x$PLATFORM_NAME" = "xDarwin"])
AM_CONDITIONAL(PLATFORM_EL, [test "x$PLATFORM_NAME" = "xRHEL" -o "x$PLATFORM_NAME" = "xCentOS"])

AC_MSG_CHECKING([architecture])
AS_IF([python -mplatform | grep -qi x86_64], [ARCH="x86_64";], [ARCH="i386";])
AC_MSG_RESULT([$ARCH])

# Setup include paths
AM_COND_IF([PLATFORM_DARWIN],
	[AM_CXXFLAGS="-I/System/Library/Frameworks/Accelerate.framework/Versions/Current/Frameworks/vecLib.framework/Versions/Current/Headers"],
	[AM_CXXFLAGS="`pkg-config --cflags protobuf`"])

dnl ---------------------------------------------------------------------------
dnl Environment checks

AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P
AC_HEADER_STDC([])

dnl Always use C11
AM_CXXFLAGS="${AM_CXXFLAGS} -std=c++11 -Wno-deprecated"

dnl ---------------------------------------------------------------------------
dnl Place this copyright notice in generated configure

AC_COPYRIGHT([Copyright (c) Solidra LLC. All rights reserved.])

dnl ---------------------------------------------------------------------------
dnl Enable/disable debugging

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],
                              [Build binaries with debugging info.])],
              [AS_IF([test "x$enable_debug" = "xyes"], 
                       [AM_CXXFLAGS="${AM_CXXFLAGS} -g -O0 -DDEBUG"],
                     [test "x$enable_debug" = "xno"], 
                       [AM_CXXFLAGS="${AM_CXXFLAGS} -DNDEBUG"])],
              [AM_CXXFLAGS="${AM_CXXFLAGS} -DNDEBUG"])

AM_CONDITIONAL(IS_DEBUGON, [echo "${CXXFLAGS}${AM_CXXFLAGS}" | grep 'DDEBUG' >/dev/null 2>/dev/null ])

dnl ---------------------------------------------------------------------------

CPP_PLUGIN="`which grpc_cpp_plugin`"
AS_IF([test "x$CPP_PLUGIN" = "x"],
	[AC_MSG_ERROR([cannot find C++ gRPC plugin])])
AC_MSG_CHECKING([gRPC plugin])
AC_MSG_RESULT([$CPP_PLUGIN])

CAFFE_CXXFLAGS=`./scripts/caffe_cxxflags.sh`
CAFFE_INCLUDE_DIR='$(top_srcdir)/deps/caffe/BUILD/caffe/install/include'
CAFFE_LIB_DIR='$(top_srcdir)/deps/caffe/BUILD/caffe/install/lib'

AM_COND_IF([PLATFORM_DARWIN],
	[OSX_BREW="`which brew`";OSX_PORT="`which port`";])

dnl https://github.com/grpc/grpc/issues/7830
dnl Have to statically link on OSX

AS_IF([test "x$OSX_BREW" != "x" -a "x$OSX_PORT" != "x" ],
    [AC_MSG_WARN([homebrew and mac ports installed - assuming homebrew])],
)

AS_IF([test "x$OSX_BREW" != "x" ],
        [AM_LDFLAGS="-L/usr/local/opt/openssl/lib /usr/local/lib/libgrpc++.a /usr/local/lib/libgrpc.a -lcrypto -lssl -lgflags -lglog -lprotobuf -lpthread -lboost_filesystem -lboost_system"],
      [test "x$OSX_PORT" == "x" ],
        [AM_LDFLAGS="-lgrpc++ -lgrpc `pkg-config --libs protobuf` -lcrypto -lssl -lgflags -lglog -lboost_filesystem -lboost_system"],
      [AC_MSG_ERROR([Mac ports not supported. You will need to add a solution for gRPC bug 7830])]
)

dnl ---------------------------------------------------------------------------
AC_CONFIG_FILES(
    [
    Makefile
    include/Makefile
    src/main/cpp/lucida/Makefile
    src/test/cpp/lucida/Makefile
    ])

AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(CPP_PLUGIN)
AC_SUBST(CAFFE_CXXFLAGS)
AC_SUBST(CAFFE_LIB_DIR)
AC_SUBST(CAFFE_INCLUDE_DIR)
AC_OUTPUT

