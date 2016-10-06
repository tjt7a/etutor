if python -mplatform | grep -qi 'darwin'; then
	[ -e BUILD/caffe/install/lib/libcaffe.dylib ] || exit 1
else
	[ -e BUILD/caffe/install/lib/libcaffe.so ] || exit 1
fi
exit 0
