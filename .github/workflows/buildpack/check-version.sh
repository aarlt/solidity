#!/usr/bin/env bash
set -e

if [ -z "${IMAGE_NAME}" ] || [ -z "${IMAGE_VARIANT}" ] || [ -z "${GITHUB_TOKEN}" ] || [ -z "${GITHUB_ACTOR}" ] || [ -z "${GITHUB_DOCKER_REPOSITORY}" ]; then
  echo "\${IMAGE_NAME}, \${IMAGE_VARIANT}, \${GITHUB_TOKEN}, \${GITHUB_ACTOR} and \${GITHUB_DOCKER_REPOSITORY} need to be defined."
  exit 1
fi

VERSION=$(grep -e 'LABEL version=\".*\"' "scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}" | awk -F"\"" '{ print $2 }')
echo "::set-env name=VERSION::${VERSION}"

DOCKER_IMAGE_ID="$IMAGE_NAME-$IMAGE_VARIANT"
if [ "${IMAGE_VARIANT}" == "emscripten" ]; then
  EMSDK_VERSION=$(grep "emscripten/emsdk" scripts/docker/buildpack/Dockerfile.emscripten | cut -f 2 -d ' ' | cut -f 2 -d ':')
  DOCKER_IMAGE_ID="${DOCKER_IMAGE_ID}-${EMSDK_VERSION}"
fi

echo "${GITHUB_TOKEN}" | docker login docker.pkg.github.com -u "${GITHUB_ACTOR}" --password-stdin
VERSION_EXIST=$(docker pull -q "${GITHUB_DOCKER_REPOSITORY}/${DOCKER_IMAGE_ID}:${VERSION}" || echo false && true)
docker logout docker.pkg.github.com

if [ "${VERSION_EXIST}" != "false" ]; then
  echo ""
  echo "ERROR: version '${VERSION}' already exist for '${DOCKER_IMAGE_ID}'. Aborting."
  echo ""

  false
fi
