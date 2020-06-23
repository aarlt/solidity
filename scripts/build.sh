#!/usr/bin/env bash
set -e

ROOTDIR="$(dirname "$0")/.."
if [[ "$( cd "$(dirname "$0")" ; pwd -P )" == *".github"* ]]
then
    ROOTDIR="/root/project"
fi

BUILDDIR="${ROOTDIR}/build"

if [[ $# -eq 0 ]]; then
    BUILD_TYPE=Release
else
    BUILD_TYPE="$1"
fi

if [[ "$(git tag --points-at HEAD 2>/dev/null)" == v* ]]; then
	touch "${ROOTDIR}/prerelease.txt"
fi

mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"

cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${@:2}"
make -j2

if [[ "${CI}" == "" ]]; then
	echo "Installing ..."
	sudo make install
fi
