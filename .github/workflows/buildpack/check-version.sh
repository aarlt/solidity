#!/bin/bash
set -e

if [ -z "${IMAGE_NAME}" ] || [ -z "${IMAGE_VARIANT}" ] || [ -z "${GITHUB_TOKEN}" ] || [ -z "${GITHUB_ACTOR}" ] || [ -z "${GITHUB_DOCKER_REPOSITORY}" ]; then
  echo "\${IMAGE_NAME}, \${IMAGE_VARIANT}, \${GITHUB_TOKEN}, \${GITHUB_ACTOR} and \${GITHUB_DOCKER_REPOSITORY} need to be defined."
  exit 1
fi

VERSION=$(grep -e 'LABEL version=\".*\"' "scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}" | awk -F"\"" '{ print $2 }')
echo "::set-env name=VERSION::${VERSION}"

echo "${GITHUB_TOKEN}" | docker login docker.pkg.github.com -u "${GITHUB_ACTOR}" --password-stdin
VERSION_EXIST=$(docker pull -q "${GITHUB_DOCKER_REPOSITORY}/${IMAGE_NAME}-${IMAGE_VARIANT}:${VERSION}" || echo false && true)
docker logout docker.pkg.github.com

if [ "${VERSION_EXIST}" != "false" ]; then
  echo ""
  echo "ERROR: version '${VERSION}' already exist for '${IMAGE_NAME}-${IMAGE_VARIANT}'. Aborting."
  echo ""

  false
fi
