#!/bin/bash

# Exit on error
set -e

# ALARA executable
ALARA=../bin/alara

# Print version information
${ALARA} -V

# Setup output directories
rm -rf   output dump_files
mkdir -p output dump_files

# Run samples
samples=sample?
for sample in ${samples}; do
  echo ${sample}
  ${ALARA} -v 2 -t output/${sample}.tree ${sample} > output/${sample}.out
done
