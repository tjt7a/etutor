mkdir -p BUILD &&
	cd BUILD ||
	die "directory access"

[ -e Python-${PYTHON_VERSION}.tgz ] || 
	wget http://www.python.org/ftp/python/${PYTHON_VERSION}/Python-${PYTHON_VERSION}.tgz ||
	die "could not download python $PYTHON_VERSION"

LOCALPY=localpython`echo "$PYTHON_VERSION" | sed 's/\./_/g'`

[ ! -d Python-${PYTHON_VERSION} ] &&
	tar -zxf Python-${PYTHON_VERSION}.tgz ||
	die "untar python $PYTHON_VERSION"

mkdir -p $BUILDPATH/ENV/$LOCALPY &&
	cd Python-$PYTHON_VERSION &&
	./configure --prefix=$BUILDPATH/ENV/$LOCALPY &&
	make &&
	make install ||
	die "build failed"

cd $BUILDPATH/ENV &&
	virtualenv vpython -p $LOCALPY/bin/python2.7 ||
	die "cannot enter python virtualenv"

source vpython/bin/activate
pip install --upgrade distribute
pip install --upgrade pip
pip install -r $ROOTPATH/../tools/python_requirements.txt
pip install grpcio
pip install grpcio-tools
pip install --upgrade numpy
deactivate
