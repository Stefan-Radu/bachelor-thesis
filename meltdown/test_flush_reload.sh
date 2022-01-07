#!/bin/bash

cnt=0

for i in {1..20}; do
  ./flushreload 1>/dev/null
  if [[ $? -eq 94 ]]; then
    cnt=$(($cnt + 1))
  fi
done

echo "Ran 20 times. Got the secret $cnt times"
