#!/bin/bash

#for file in $(cat allfiles); do
    #printf "downloading $file; "
    #wget "ftp://ftp.wwpdb.org/pub/pdb/data/structures/all/pdb/$file" -O "./pdb/$file" --quiet
    #./mkplg.sh "./pdb/$file"
#done

cat allfiles | parallel -P 4 ./mkplg.sh {}
