#!/bin/bash

for f in output/*; do
  diffs=$(diff $f output_ref/$(basename $f))
  if [ -z "${diffs}" ]; then
    echo "No diffs for $(basename $f)"
  else
    echo "Diffs for $(basename $f):"
    echo ${diffs}
  fi
done
