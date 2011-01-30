#!/bin/sh
V=`expr $(git log --no-color --pretty='format:%h' | wc -l) + 110`
while [ "${#V}" -lt 4 ]; do
  V=0$V
done
echo -n ${V:0:2}.${V:2:2}
