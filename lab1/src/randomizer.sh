#!/bin/bash
exec 3>number.txt
for i in {1..150}; do
  num=$(od -An -N1 -tu1 /dev/random)
  echo $num >&3
done
exec 3>$-
