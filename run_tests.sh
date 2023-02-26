#!/bin/bash

SOURCE_DIR=$(dirname -- ${BASH_SOURCE[0]})
BUILD_DIR=$SOURCE_DIR/.build
TEST_DIR=$SOURCE_DIR/tests

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR

pushd $BUILD_DIR

cmake ../$TEST_DIR
cmake --build .

if [ $? -eq 0 ]; then
  find . -maxdepth 1 -type f -perm /111 -name 'regit_test*' -exec {} \;
else
  echo "Compilation failed."
  exit 1
fi

popd