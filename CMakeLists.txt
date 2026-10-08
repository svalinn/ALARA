project(ALARA)
cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

# Default to a release build
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "CMAKE_BUILD_TYPE not specified, defaulting to Release")
  set(CMAKE_BUILD_TYPE Release)
endif ()

# Version strings  
set(PACKAGE_VERSION 2.9.2)
set(PACKAGE_STRING "ALARA ${PACKAGE_VERSION}")

# Location of data directory
set(DFLT_DATADIR ${CMAKE_INSTALL_PREFIX}/data)
set(DFLT_XSDIR   ${CMAKE_INSTALL_PREFIX}/data)

set(GIT_HASH_HEADER "${CMAKE_CURRENT_BINARY_DIR}/git_hash.h")

add_custom_target(update_git_hash ALL
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DOUTPUT_FILE=${GIT_HASH_HEADER}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/FindGitHash.cmake
)

# Build ALARA executable
add_subdirectory(src)

# Build other ALARA tools
add_subdirectory(tools)

# Install data files
add_subdirectory(data)

# Install sample files
add_subdirectory(sample)