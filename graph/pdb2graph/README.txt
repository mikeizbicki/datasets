PDB-to-Graph Program

Written by Larry Holder (holder@cse.uta.edu)
Department of Computer Science and Engineering
University of Texas at Arlington

The pdb2graph program converts a Protein Data Bank (PDB) file to a graph in
SUBDUE format (see http://ailab.uta.edu/subdue).  This directory contains
several files from the Protein Data Bank at http://www.rcsb.org/pdb.  There
is also a program (pdb2graph.c) that converts PDB files to Subdue graph
files.  To compile the program on UNIX, use the following command.  See the
top of the source code for a description of the program.

  cc -o pdb2graph pdb2graph.c -lm

The program takes two arguments, the input PDB file and the output Subdue
file.  Two graph files have already been generated for Hemoglobin (pdb1hga)
and DNA (pdb7icg) and are stored in the corresponding .g files.  These files
were generated with the following commands.

  pdb2graph pdb1hga.ent psb1hga.g
  psb2graph pdb7icg.ent pdb7icg.g

Note that each atom vertex entry in the graph files also contains a comment
with the x, y and z coordinate for the 3D location of the atom (in
angstroms).
