
AUTOMAKE_OPTIONS=foreign subdir-objects

SUBDIRS= \
    include \
    src/main/cpp/lucida \
    src/test/cpp/lucida

EXTRADIST=\
	settings.gradle \
	gradlew \
	gradlew.bat \
	README.md

# Handle automake/make conflicts
TARGETDIR=$(strip $(if $(findstring $(abs_top_srcdir), $(abs_top_builddir)), $(abs_top_builddir)/bin, $(abs_top_builddir)))

.PHONY: test init reinit

test: all
	cd $(top_builddir)/src/test/cpp/lucida && $(MAKE) test
	gradle test
	
# Initialize build - only need to do once
init:
	cd deps && $(MAKE)

reinit: clean
	cd deps && $(MAKE) clean && $(MAKE)
	autoreconf -f

all-local:
	gradle build -x test

clean-local:
	gradle clean

