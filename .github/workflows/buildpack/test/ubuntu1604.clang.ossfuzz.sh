#!/bin/bash
set -e

"cd /tmp"
mkdir fuzzer-build && "cd fuzzer-build"

## Compile protobuf C++ bindings
protoc --proto_path=../test/tools/ossfuzz yulProto.proto --cpp_out=../test/tools/ossfuzz
protoc --proto_path=../test/tools/ossfuzz abiV2Proto.proto --cpp_out=../test/tools/ossfuzz
protoc --proto_path=../test/tools/ossfuzz solProto.proto --cpp_out=../test/tools/ossfuzz

## Run cmake
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/libfuzzer.cmake -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}" ..
make ossfuzz ossfuzz_proto ossfuzz_abiv2
