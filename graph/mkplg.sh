#!/bin/bash

file=$1
dir="pdb"
pdb_id=$(cut -c4-7 <<< $file )

printf "processing $pdb_id; "

printf "downloading; "
wget "ftp://ftp.wwpdb.org/pub/pdb/data/structures/all/pdb/$file" -O "./$dir/$file" --quiet

startdir=$(pwd)
cd "pdb"
basefile="$pdb_id"
mv "$(basename $1)" "${basefile}.pdb.gz"
gunzip "${basefile}.pdb.gz"

printf "making dssp; "
$startdir/dssp-2.2.1/mkdssp "${basefile}.pdb" "${basefile}.dssp" > /dev/null 2> /dev/null

if [ $? = 0 ]; then
    printf "making plg; "
    java -jar $startdir/vplg/plcc.jar ${basefile} -g f -w > log.vplg 2>&1
    #java -jar $startdir/vplg/plcc.jar ${basefile} -g albelig --dont-write-images > log.vplg 2>&1

    printf "cleaning; "
    rm *$pdb_id*.el_ntl
    rm *$pdb_id*.el_edges
    rm *$pdb_id*.gml
    rm *$pdb_id*.gv
    rm *$pdb_id*.kavosh
    rm *$pdb_id*.tgf
    rm $basefile.dssp
else
    printf "dssp couldn't be created"
fi

rm $basefile.pdb
touch $basefile

echo
