# Install sample files
file(GLOB SAMPLE_FILES sample?)
install(FILES ${SAMPLE_FILES} DESTINATION sample)

# Install scripts
install(PROGRAMS run_samples.sh diff_output.sh DESTINATION sample)

# Install sample data directory
install(DIRECTORY data DESTINATION sample)

# Install reference output directory
install(DIRECTORY output_ref DESTINATION sample)
