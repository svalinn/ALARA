# Sample files
file(GLOB SAMPLE_FILES "sample?" "run_samples.sh")

# Install sample files
install(FILES ${SAMPLE_FILES} DESTINATION sample)

# Install sample data directory
install(DIRECTORY data DESTINATION sample)
