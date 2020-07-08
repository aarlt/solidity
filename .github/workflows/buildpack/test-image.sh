#!/bin/bash
set -e

if [ -z "${IMAGE_NAME}" ]; then
  echo "\${IMAGE_NAME} need to be defined."
  exit 1
fi

SCRIPT=/tmp/scripts/build.sh
if [ -f ".github/workflows/buildpack/test/${IMAGE_NAME}.sh" ]; then
  SCRIPT=/tmp/.github/workflows/buildpack/test/${IMAGE_NAME}.sh
fi

docker run -v "${PWD}:/tmp" -e CI=1 "${IMAGE_NAME}" "${SCRIPT}"
