include(ExternalProject)

ExternalProject_Add(nlohmann-json
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/nlohmann/json"
    DOWNLOAD_NAME json.hpp
    DOWNLOAD_NO_EXTRACT 1
    URL https://github.com/nlohmann/json/releases/download/v3.1.2/json.hpp
    URL_HASH SHA256=fbdfec4b4cf63b3b565d09f87e6c3c183bdd45c5be1864d3fcb338f6f02c1733
    CMAKE_COMMAND true
    BUILD_COMMAND true
    INSTALL_COMMAND true
)

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/deps/nlohmann)
