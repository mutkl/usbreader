#!/bin/bash

rm -rf raw_data*.txt
rm -rf coincidence_data*.txt
rm -rf coincidence_data*.root
rm -rf data.root
rm -rf data_params.txt
rm -rf result.root
rm -rf detector.eps
list=`ls output/`
for dir in $list; do
    echo " rm -rf " $dir "? [N/y/s]"
    read a
    if [ "$a" != "Y" ] && [ "$a" != "y" ] && [ "$a" != "s" ]
    then
	continue
    fi
    rm -rf output/$dir
done

