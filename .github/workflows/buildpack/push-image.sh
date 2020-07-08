#!/bin/bash
set -e

if [ -z "${IMAGE_NAME}" ] || [ -z "${IMAGE_VARIANT}" ] || [ -z "${GITHUB_TOKEN}" ] || [ -z "${GITHUB_ACTOR}" ] || [ -z "${GITHUB_DOCKER_REPOSITORY}" ]; then
  echo "\${IMAGE_NAME}, \${IMAGE_VARIANT}, \${GITHUB_TOKEN}, \${GITHUB_ACTOR} and \${GITHUB_DOCKER_REPOSITORY} need to be defined."
  exit 1
fi

echo "${GITHUB_TOKEN}" | docker login docker.pkg.github.com -u "${GITHUB_ACTOR}" --password-stdin

DOCKER_IMAGE_ID="$GITHUB_DOCKER_REPOSITORY/$IMAGE_NAME-$IMAGE_VARIANT"
if [ "${IMAGE_VARIANT}" == "emscripten" ]; then
  EMSDK_VERSION=$(grep "emscripten/emsdk" scripts/docker/buildpack/Dockerfile.emscripten | cut -f 2 -d ' ' | cut -f 2 -d ':')
  DOCKER_IMAGE_ID="${DOCKER_IMAGE_ID}-${EMSDK_VERSION}"
fi
VERSION=$(docker inspect --format='{{.Config.Labels.version}}' "${IMAGE_NAME}")

docker tag "${IMAGE_NAME}" "${DOCKER_IMAGE_ID}:${VERSION}"
docker push "${DOCKER_IMAGE_ID}:${VERSION}"

REPO_DIGEST=$(docker inspect --format='{{.RepoDigests}}' "${DOCKER_IMAGE_ID}:${VERSION}")
echo "::set-env name=DOCKER_IMAGE::${DOCKER_IMAGE_ID}:${VERSION}"
echo "::set-env name=DOCKER_REPO_DIGEST::${REPO_DIGEST}"

docker tag "${IMAGE_NAME}" "${DOCKER_IMAGE_ID}:latest"
docker push "${DOCKER_IMAGE_ID}:latest"

docker logout docker.pkg.github.com
