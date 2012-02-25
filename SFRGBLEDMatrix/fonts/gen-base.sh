#!/bin/bash

for a in 0 1 2 3 4 5 6 7 8 9 A B C D E F
do
  for b in 0 1 2 3 4 5 6 7 8 9 A B C D E F
  do
    echo -n "# 0x$a$b "
    echo -e \\x$a$b
    echo 000
    echo 000
    echo 000
    echo 000
   done
done

