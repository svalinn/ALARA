# CMake file in the DataLib direcotry within the src directory

FILE(GLOB h_files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
FILE(GLOB c_files "${CMAKE_CURRENT_SOURCE_DIR}/*.C")
FILE(GLOB cpp_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
SET(DATA_LIB_SOURCES ${c_files} ${cpp_files})


# Add the library
add_library(DataLib SHARED ${DATA_LIB_SOURCES})
