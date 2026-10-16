# Source files
file(GLOB SRC_FILES "*.C" "DataLib/*.C" "DataLib/*.cpp")

# Create some header files
configure_file(config.h.in       config.h      )
configure_file(dflt_datadir.h.in dflt_datadir.h)

# Location of header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR})

# ALARA executable
add_executable(alara ${SRC_FILES})

# Make sure the Git hash is generated before compiling ALARA
add_dependencies(alara update_git_hash)

install(TARGETS alara DESTINATION bin)
