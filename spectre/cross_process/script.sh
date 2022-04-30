#!/bin/bash

for i in {0..40}; do
  for j in range{0..99}; do
    ./victim $i >> /dev/null
  done
done
