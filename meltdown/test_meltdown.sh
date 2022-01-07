#!/bin/bash

gcc -march=native MeltdownExperiment.c -o meltexp

cnt=0

for i in {1..500}; do
  ./meltexp 1>/dev/null
  val=$?
  if [[ $val -ne 0 ]]; then
    echo $val
    break
  fi
done
