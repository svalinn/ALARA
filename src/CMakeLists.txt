# src directory CMake file

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
configure_file(version.h.in "${CMAKE_CURRENT_SOURCE_DIR}/config.h")

FILE(GLOB h_files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
FILE(GLOB c_files "${CMAKE_CURRENT_SOURCE_DIR}/*.C")
SET(EXECUTABLES ${c_files})

# Add the Data Library
add_subdirectory(DataLib)

# Add all source files
add_executable(alara ${EXECUTABLES})
target_link_libraries(alara DataLib)
