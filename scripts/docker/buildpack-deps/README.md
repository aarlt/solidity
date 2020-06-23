# buildpack-deps docker images

The `buildpack-deps` docker images are used to compile and test solidity within our CI.

## GitHub Workflow

The creation of the images are triggered by different workflows. For each resulting
`buildpack-deps` docker image a workflow is defined in `.github/workflows/buildpack-deps`.
The workflows get triggered, if the workflow file itself, or the Dockerfiles `scripts/docker/buildpack-deps/Dockerfile.*`
where changed within a PR.

### Versioning

The version of the docker images can be defined within the Dockerfile with `LABEL version`. A new docker image
will only be created and pushed, if the new version is incremented by `1` compared with the version of the Dockerfile
located in `develop`.

### Build, Test & Push

If the version check was successful, the docker image will be build using the Dockerfile located in
`scripts/docker/buildpack-deps/Dockerfile.*`.

The resulting docker image will be tested by executing
the corresponding `scripts/ci/buildpack-deps_test_*` scripts. These scripts are normally symlinked to `scripts/ci/build.sh`,
except for the `buildpack-deps-ubuntu1604.clang.ossfuzz` docker image, that is symlinked to `scripts/ci/build_ossfuzz.sh`.
These scripts `scripts/ci/build.sh` and `scripts/ci/build_ossfuzz.sh` are also used by CircleCI, see `.circleci/config.yml`.

If the tests passed successfully, the docker image will get tagged by the version defined within the corresponding `Dockerfile`.
Finally, a comment will be added to the PR that contains the full repository, version and repository digest
of the freshly created docker image.