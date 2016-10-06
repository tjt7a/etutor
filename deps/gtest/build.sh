if [ -z "$THREADS" ]; then
	THREADS=4
fi

if [ ! -d BUILD ]; then
	git clone https://github.com/google/googletest.git BUILD &&
	cd BUILD &&
	git checkout tags/release-$RELEASE &&
	cd ../ ||
	die "could not download googletest $RELEASE"
fi

cd BUILD &&
	cmake . &&
	make -j$THREADS ||
	die "build failed"
