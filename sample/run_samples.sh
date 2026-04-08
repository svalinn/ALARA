#!/bin/bash

# Exit on error
set -e

# ALARA executable
ALARA=../bld/src/alara

# Print version information
${ALARA} -V

# Setup output directories
rm -rf   output dump_files
mkdir -p output dump_files

# Run samples
for sample in $(ls sample* | sort -V); do
  echo ${sample}
  ${ALARA} -v 3 -t output/${sample}.tree ${sample} > output/${sample}.out 2>&1
done
