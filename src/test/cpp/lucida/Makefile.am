bin_PROGRAMS = lucida_test

lucida_test_SOURCES = \
	utils/path_test.cpp \
	handler.cpp \
	handler.h \
	client_server.cpp

lucida_test_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../deps/gtest/BUILD/googletest/include 

lucida_test_LDFLAGS = $(top_builddir)/src/main/cpp/lucida/liblucida.la \
		$(top_srcdir)/../deps/gtest/BUILD/googlemock/gtest/libgtest.a \
		$(top_srcdir)/../deps/gtest/BUILD/googlemock/gtest/libgtest_main.a $(AM_LDFLAGS)

.PHONY: test

test:
	./lucida_test
