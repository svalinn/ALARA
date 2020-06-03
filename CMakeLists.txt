# Top level CMake file

# Set the minimum version of CMake required to run properly
cmake_minimum_required(VERSION 3.0.0)

# Set the project name
project(ALARA VERSION 2.7.1)

# Add all subdirectories
# add_subdirectory(data)
# add_subdirectory(developer-info)
# add_subdirectory(doc)
add_subdirectory(sample)
add_subdirectory(src)
# add_subdirectory(tools)


