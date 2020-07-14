#!/usr/bin/env bash
set -e

function error() {
  echo >&2 "ERROR: ${1} Aborting." && false
}

function warning() {
  echo >&2 "WARNING: ${1}"
}

check_dockerfile_was_changed() {
  echo "-- check_dockerfile_was_changed"

  [[ $# == 2 ]] || error "Expected exactly 2 parameters: 'check_dockerfile_was_changed <IMAGE_NAME> <IMAGE_VARIANT>'"
  local IMAGE_NAME="${1}"
  local IMAGE_VARIANT="${2}"
  local DOCKERFILE="scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}"

  # exit, if the dockerfile was not changed.
  if [ "$(git diff --name-only origin/docker-workflow HEAD -- "${DOCKERFILE}")" ]; then
    echo "${DOCKERFILE} was not changed. Nothing to do."
    exit 0
  fi
}

check_version() {
  echo "-- check_version"

  [[ $# == 2 ]] || error "Expected exactly 2 parameters: 'check_version <IMAGE_NAME> <IMAGE_VARIANT>'"
  local IMAGE_NAME="${1}"
  local IMAGE_VARIANT="${2}"
  local DOCKERFILE="scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}"

  local PREV_VERSION
  PREV_VERSION=$(git diff origin/develop HEAD -- "${DOCKERFILE}" | grep -e '^\s*-LABEL\s\+version=".*"\s*$' | awk -F'"' '{ print $2 }')
  local NEXT_VERSION
  NEXT_VERSION=$(git diff origin/develop HEAD -- "${DOCKERFILE}" | grep -e '^\s*+LABEL\s\+version=".*"\s*$' | awk -F'"' '{ print $2 }')

  [[ $NEXT_VERSION != "" ]] || error "No version label defined in Dockerfile. You may need to add 'LABEL version' in '${DOCKERFILE}'."

  [[ $PREV_VERSION != "" ]] || {
    warning "no previous version found. Will set \$PREV_VERSION = 0."
    PREV_VERSION=0
  }

  if [[ $((PREV_VERSION + 1)) != $((NEXT_VERSION)) ]]; then
    error "Version label in Dockerfile was not incremented. You may need to change 'LABEL version' in '${DOCKERFILE}'."
  fi
}

build_docker() {
  echo "-- build_docker"

  [[ $# == 2 ]] || error "Expected exactly 2 parameters: 'build_docker <IMAGE_NAME> <IMAGE_VARIANT>'."
  local IMAGE_NAME="${1}"
  local IMAGE_VARIANT="${2}"

  # This is a workaround: we run `docker build` twice to prevent the `layer does not exist` problem.
  # See https://github.com/moby/moby/issues/37965.
  docker build "scripts/docker/${IMAGE_NAME}" --file "scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}" --tag "${IMAGE_NAME}" ||
    docker build "scripts/docker/${IMAGE_NAME}" --file "scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}" --tag "${IMAGE_NAME}"
}

test_docker() {
  echo "-- test_docker @ '${PWD}'"

  [[ $# == 2 ]] || error "Expected exactly 3 parameters: 'test_docker <IMAGE_NAME> <IMAGE_VARIANT>'."
  local IMAGE_NAME="${1}"
  local IMAGE_VARIANT="${2}"

  docker run --rm --volume "${PWD}:/root/project" "${IMAGE_NAME}" "/root/project/scripts/ci/${IMAGE_NAME}_test_${IMAGE_VARIANT}.sh"
}

push_docker() {
  echo "-- push_docker"

  [[ $# == 3 ]] || error "Expected exactly 2 parameters: 'push_docker <IMAGE_NAME> <IMAGE_VARIANT> <DOCKER_REPOSITORY>'."
  local IMAGE_NAME="${1}"
  local IMAGE_VARIANT="${2}"
  local DOCKER_REPOSITORY="${3}"

  local VERSION
  VERSION=$(docker inspect --format='{{.Config.Labels.version}}' "${IMAGE_NAME}")
  local DOCKER_IMAGE_ID="${DOCKER_REPOSITORY}/${IMAGE_NAME}-${IMAGE_VARIANT}"

  docker tag "${IMAGE_NAME}" "${DOCKER_IMAGE_ID}:${VERSION}"
  docker push "${DOCKER_IMAGE_ID}:${VERSION}"

  local REPO_DIGEST
  REPO_DIGEST=$(docker inspect --format='{{.RepoDigests}}' "${DOCKER_IMAGE_ID}:${VERSION}")

  docker tag "${IMAGE_NAME}" "${DOCKER_IMAGE_ID}:latest"
  docker push "${DOCKER_IMAGE_ID}:latest"

  echo "::set-env name=DOCKER_IMAGE::${DOCKER_IMAGE_ID}:${VERSION}"
  echo "::set-env name=DOCKER_REPO_DIGEST::${REPO_DIGEST}"
}

[[ $# == 3 ]] || error "Expected exactly 3 parameters: '${0} <IMAGE_NAME> <IMAGE_VARIANT> <DOCKER_REPOSITORY>'."

IMAGE_NAME="${1}"
IMAGE_VARIANT="${2}"
DOCKER_REPOSITORY="${3}"

check_dockerfile_was_changed "${IMAGE_NAME}" "${IMAGE_VARIANT}"

# These functions will only get called, if the corresponding Dockerfile was changed.
check_version "${IMAGE_NAME}" "${IMAGE_VARIANT}"
build_docker "${IMAGE_NAME}" "${IMAGE_VARIANT}"
test_docker "${IMAGE_NAME}" "${IMAGE_VARIANT}"
push_docker "${IMAGE_NAME}" "${IMAGE_VARIANT}" "${DOCKER_REPOSITORY}"
