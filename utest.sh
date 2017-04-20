#!/bin/bash 
echo 'wtf'
find .build -name 'utest_debug' -print0 | while read -d $'\0' f;
do
    echo $f
    if ! exec $f; then 
        exit $? 
    fi
done
find .build -name 'utest_release' -print0 | while read -d $'\0' f;
do
    if ! exec $f; then 
        exit $? 
    fi
done
