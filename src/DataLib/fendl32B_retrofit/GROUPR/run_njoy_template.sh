#!/bin/bash

# Check if NJOY is installed and accessible
if ! command -v njoy &> /dev/null
then
    echo "NJOY could not be found. Please make sure it is installed and added to your PATH."
    exit 1
fi

# Define the input files
TAPE20="<ENDF_PATH>"
TAPE21="<PENDF_PATH>"
INPUT="groupr.inp"
OUTPUT="groupr.out"

# Check if input files exist
if [ ! -f "$TAPE20" ]; then
    echo "ENDF file not found!"
    exit 1
fi

if [ ! -f "$TAPE21" ]; then
    echo "PENDF file not found!"
    exit 1
fi

if [ ! -f "$INPUT" ]; then
    echo "Input file not found!"
    exit 1
fi

# Run NJOY with the input file
echo "Running NJOY..."
njoy < "$INPUT" > "$OUTPUT"

# Check if NJOY ran successfully
if [ $? -eq 0 ]; then
    echo "NJOY ran successfully. Output written to /output."
else
    echo "NJOY encountered an error. Check /output for details."
    exit 1
fi
