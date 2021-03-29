#!/bin/bash

files_ref=output_ref/*
for f_ref in $files_ref; do
  f=output/$(basename ${f_ref})
  diffs=$(diff ${f} ${f_ref})
  if [ -z "${diffs}" ]; then
    echo "No diffs for ${f}"
  else
    echo "Diffs for ${f}:"
    echo ${diffs}
  fi
done
