#!/usr/bin/env bash
set -e

if [ -z "${IMAGE_NAME}" ] || [ -z "${IMAGE_VARIANT}" ]; then
  echo "\${IMAGE_NAME} and \${IMAGE_VARIANT} need to be defined."
  exit 1
fi

SCRIPT=/tmp/scripts/build.sh
if [ -f ".github/workflows/buildpack/test/${IMAGE_VARIANT}.sh" ]; then
  SCRIPT=/tmp/.github/workflows/buildpack/test/${IMAGE_VARIANT}.sh
fi

if [ "${IMAGE_VARIANT}" == "emscripten" ]; then
  # build_emscripten.sh will setup docker
  echo ">>> scripts/build_emscripten.sh emscripten_build \"${IMAGE_NAME}\""
  scripts/build_emscripten.sh emscripten_build "${IMAGE_NAME}"
else
  # run script within docker
  echo ">>> docker run -v \"${PWD}:/tmp\" -e CI=1 -e CC=${CC} -e CXX=${CXX} \"${IMAGE_NAME}\" \"${SCRIPT}\""
  docker run -v "${PWD}:/tmp" -e CI=1 -e CC="${CC}" -e CXX="${CXX}" "${IMAGE_NAME}" "${SCRIPT}"
fi
