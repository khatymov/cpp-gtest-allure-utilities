cmake_minimum_required(VERSION 3.15)
project(JSONAdapterInterface LANGUAGES CXX)

# Header-only "library" with your interfaces
add_library(JSONAdapterInterface INTERFACE)

target_include_directories(JSONAdapterInterface
    INTERFACE
        # During build: add this folder to the include path
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        # After install: headers will be under include/
        $<INSTALL_INTERFACE:include>
)

# Optional, but nice to enforce
target_compile_features(JSONAdapterInterface INTERFACE cxx_std_17)

install(DIRECTORY JSONAdapterInterface
        DESTINATION include)

install(TARGETS JSONAdapterInterface
        EXPORT JSONAdapterInterfaceTargets)

install(EXPORT JSONAdapterInterfaceTargets
        NAMESPACE JSONAdapter::
        DESTINATION lib/cmake/JSONAdapterInterface)
