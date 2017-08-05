#!/bin/bash -e
mkdir -p package
cp README.md package/README
cp -r PACKAGES src online-runner install package/
rm -rf package/src/build
TAR_NAME=icfp-5022e483-705c-4611-a32b-4326dde5e643.tar.gz
cd package
tar -czf $TAR_NAME *
mv $TAR_NAME ../
cd -
rm -rf package
md5 $TAR_NAME
