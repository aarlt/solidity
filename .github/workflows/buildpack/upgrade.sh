#!/usr/bin/env bash
set -e

check_parameters() {
  echo "-- check_parameters"

  if [ -z "${IMAGE_NAME}" ] || [ -z "${IMAGE_VARIANT}" ] || [ -z "${GITHUB_TOKEN}" ] || [ -z "${GITHUB_ACTOR}" ] || [ -z "${GITHUB_DOCKER_REPOSITORY}" ]; then
    echo "\${IMAGE_NAME}, \${IMAGE_VARIANT}, \${GITHUB_TOKEN}, \${GITHUB_ACTOR} and \${GITHUB_DOCKER_REPOSITORY} need to be defined."

    false
  fi
}

check_version() {
  echo "-- check_version"

  git fetch
  git branch
  DOCKERFILE="scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}"
  PREV_VERSION=$(git diff origin/develop HEAD -- "${DOCKERFILE}" | grep -e "-LABEL version=\".*\"" | awk -F"\"" '{ print $2 }')
  NEXT_VERSION=$(git diff origin/develop HEAD -- "${DOCKERFILE}" | grep -e "+LABEL version=\".*\"" | awk -F"\"" '{ print $2 }')

  if [ -z "${NEXT_VERSION}" ]; then
    echo ""
    echo "ERROR: No version label defined in Dockerfile. You may need to add 'LABEL version' in '${DOCKERFILE}'. Aborting."
    echo ""

    false
  fi

  if [ -z "${PREV_VERSION}" ]; then
    PREV_VERSION=$((NEXT_VERSION - 1))
    echo ""
    echo "WARNING: no previous version found. Will set \$PREV_VERSION = $PREV_VERSION."
    echo ""
  fi

  if [[ $((PREV_VERSION + 1)) != $((NEXT_VERSION)) ]]; then
    echo ""
    echo "ERROR: Version label in Dockerfile was not incremented. You may need to change 'LABEL version' in '${DOCKERFILE}'. Aborting."
    echo ""

    false
  fi
}

build_docker() {
  echo "-- build_docker"

  docker build "scripts/docker/${IMAGE_NAME}" --file "scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}" --tag "${IMAGE_NAME}"
}

test_docker() {
  echo "-- test_docker"

  docker run -v "${PWD}:/root/project" -e CI=1 -e CC="${CC}" -e CXX="${CXX}" "${IMAGE_NAME}" "/root/project/.github/workflows/${IMAGE_NAME}/test-${IMAGE_VARIANT}.sh"
}

push_docker() {
  echo "-- push_docker"

  VERSION=$(docker inspect --format='{{.Config.Labels.version}}' "${IMAGE_NAME}")
  DOCKER_IMAGE_ID="${GITHUB_DOCKER_REPOSITORY}/${IMAGE_NAME}-${IMAGE_VARIANT}"

  docker tag "${IMAGE_NAME}" "${DOCKER_IMAGE_ID}:${VERSION}"
  docker push "${DOCKER_IMAGE_ID}:${VERSION}"

  REPO_DIGEST=$(docker inspect --format='{{.RepoDigests}}' "${DOCKER_IMAGE_ID}:${VERSION}")

  docker tag "${IMAGE_NAME}" "${DOCKER_IMAGE_ID}:latest"
  docker push "${DOCKER_IMAGE_ID}:latest"

  echo "::set-env name=DOCKER_IMAGE::${DOCKER_IMAGE_ID}:${VERSION}"
  echo "::set-env name=DOCKER_REPO_DIGEST::${REPO_DIGEST}"
}

check_parameters
check_version
build_docker
test_docker
push_docker
