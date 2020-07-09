#!/usr/bin/env bash
set -e

ROOTDIR="$(dirname "$0")/.."
BUILDDIR="${ROOTDIR}/build"

if test ! -z "$1"; then
	BUILDDIR="$1"
fi

echo ">>>> ${BUILDDIR}"

mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"

echo ">>>> ${PWD}"

ls ../

echo "-----------"

protoc --proto_path=../test/tools/ossfuzz yulProto.proto --cpp_out=../test/tools/ossfuzz
protoc --proto_path=../test/tools/ossfuzz abiV2Proto.proto --cpp_out=../test/tools/ossfuzz
protoc --proto_path=../test/tools/ossfuzz solProto.proto --cpp_out=../test/tools/ossfuzz
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/libfuzzer.cmake -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}" "$CMAKE_OPTIONS"
make ossfuzz ossfuzz_proto ossfuzz_abiv2 -j4
