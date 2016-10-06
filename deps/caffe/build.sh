if [ -z "$THREADS" ]; then
	THREADS=4
fi

# git checkout tags/v0.9999 or ipa
if [ ! -d BUILD/caffe ]; then
	mkdir -p BUILD &&
		cd BUILD &&
		git clone https://github.com/BVLC/caffe.git caffe &&
		cd caffe &&
		cd ../../ ||
		die "could not download caffe"
fi

## cp Makefile.config.example Makefile.config &&
cd BUILD/caffe &&
	cmake . &&
	make -j$THREADS &&
	make install ||
	die "build failed"
