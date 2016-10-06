#! /bin/sh
# MongoDB
if [ ! -e /etc/apt/sources.list.d/mongodb-org-3.2.list ]; then
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv EA312927
echo "deb http://repo.mongodb.org/apt/ubuntu trusty/mongodb-org/3.2 multiverse" | tee /etc/apt/sources.list.d/mongodb-org-3.2.list
apt-get install -y mongodb-org
service mongod start
fi
exit 0
