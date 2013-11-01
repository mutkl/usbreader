#!/bin/bash

# read filename
read_filename(){
    echo " possible outputs: "
    ls output/ | awk -F "output." '{print $2}'
    echo " what output reanalyze ? "

    read filename

    if [ ! -d "$dirname$filename" ];
    then
	echo " directory " $dirname$filename " does not exist "
	exit 1
    fi

    echo will reanalyze $filename


}



filename="default"
dirname="output/output."

read_filename

cp $dirname$filename/$filename-data_params.txt data_params.txt
cp $dirname$filename/$filename-data.root data.root

root -l analyze_data.C

cp result.root $dirname$filename/

