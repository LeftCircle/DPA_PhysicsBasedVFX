cmake_minimum_required(VERSION 3.22)

project(PhysicsBasedVFX
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optional: build tests by default
include(CTest)

# ------------------------------------------------------------
# Starter library
# ------------------------------------------------------------

file(GLOB_RECURSE STARTER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/starter/base/*.C"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/starter/base/*.cpp"
)

add_library(starter STATIC ${STARTER_SOURCES})

target_include_directories(starter
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/starter/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/starter/base"
)


# ------------------------------------------------------------
# Image Processing
# ------------------------------------------------------------

set(IMAGE_PROCESSING_DEPS_ROOT
    "/home/left/local"
    CACHE PATH
    "Prefix containing include/ and lib/ for OpenImageIO/OpenEXR"
)

set(IMAGE_PROCESSING_BUILD_VIEWER OFF CACHE BOOL "" FORCE)
set(IMAGE_PROCESSING_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(IMAGE_PROCESSING_BUILD_PYTHON OFF CACHE BOOL "" FORCE)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/DPA_ImageProcessing"
    "${CMAKE_CURRENT_BINARY_DIR}/DPA_ImageProcessing"
)

# ------------------------------------------------------------
# All my things!
# ------------------------------------------------------------

file(GLOB_RECURSE PBVFX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.C"
)

file(GLOB_RECURSE PBVFX_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_library(pbvfx STATIC
    ${PBVFX_SOURCES}
    ${PBVFX_HEADERS}
)

target_include_directories(pbvfx
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(pbvfx
    PUBLIC
        starter
        image_processing
)

# ------------------------------------------------------------
# Example/demo executable
# ------------------------------------------------------------

add_executable(physics_vfx
    main.cpp
)

target_link_libraries(physics_vfx
    PRIVATE
        pbvfx
)

# ------------------------------------------------------------
# Catch2 tests
# ------------------------------------------------------------

if(BUILD_TESTING)

    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
    )

    # Prevent Catch2's own tests from being built.
    set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(Catch2)

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    add_executable(pbvfx_tests
        ${TEST_SOURCES}
    )

    target_link_libraries(pbvfx_tests
        PRIVATE
            pbvfx
            Catch2::Catch2WithMain
    )

    # Catch2's CMake test-discovery helper is in its extras directory.
    list(APPEND CMAKE_MODULE_PATH
        ${catch2_SOURCE_DIR}/extras
    )

    include(Catch)

    catch_discover_tests(pbvfx_tests)

endif()