#!/bin/bash

find . -type f \( -name *.c -o -name *.h \) > cscope.files
cscope -b -q -k
rm cscope.files cscope.in.out cscope.po.out