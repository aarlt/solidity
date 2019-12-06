if (USE_LLVM)
    if (NOT DEFINED ENV{LLVM_HOME})
        message(WARNING "$LLVM_HOME is not defined. Using default.")
        set(ENV{LLVM_DIR} /usr/local/Cellar/llvm/9.0.0_1/lib/cmake/llvm)
    else ()
        set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
    endif ()

    find_package(LLVM REQUIRED CONFIG)

endif ()
