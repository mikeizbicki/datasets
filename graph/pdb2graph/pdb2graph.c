//-------------------------------------------------------------------------
// pdb2graph.c
//
// Usage: pdb2graph <PDB input file> <Subdue graph output file>
//
// This program reads information from the given PDB file and outputs a
// Subdue-format graph file corresponding to the compound described in the
// PDB file. Specifically, the program reads in all the ATOM information
// from the first MODEL of the PDB file.  Next, the program outputs a
// vertex for each atom whose label is the element name of that atom.  The
// three-dimensional coordinates of the atom in angstroms are included in a
// comment next the atom's vertex definition. Next, for any two atoms whose
// Euclidean distance is between 0.4 and 1.9 angstroms (or 0.4 and 1.2
// angstroms if one or both of the atoms are hydrogen), the program outputs
// a "bond" edge between the vertices of the two atoms. These bond
// distances are based on the technique used in the RasMol molecular
// visualizer.
//
// Written by Larry Holder (holder@cse.uta.edu)
//
// Copyright (c) 2004, University of Texas at Arlington
//-------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define FALSE   0
#define TRUE    1
#define LINELEN 128
#define COMMENT '%'        // comment character in Subdue graph file
#define SPACE   ' '
#define END_STRING '\0'
#define ATOMLIST_SIZE_INC 1000
#define HEADER_STR "HEADER"
#define ATOM_STR   "ATOM  "
#define ENDMDL_STR "ENDMDL"
#define END_STR    "END   "
#define ELEMENT_START 13
#define ELEMENT_END   14
#define X_START 31
#define X_END   38
#define Y_START 39
#define Y_END   46
#define Z_START 47
#define Z_END   54
#define BOND_DIST_MIN 0.4
#define BOND_DIST_MAX 1.9
#define BOND_DIST_HYDROGEN_MIN 0.4
#define BOND_DIST_HYDROGEN_MAX 1.2


// Type definitions

typedef unsigned char BOOLEAN;
typedef unsigned long ULONG;

// Atom
typedef struct {
  char element[3]; // atom element name
  float x;         // atom's x position in angstroms
  float y;         // atom's y position in angstroms
  float z;         // atom's z position in angstroms
} Atom;

// Atom list
typedef struct {
  ULONG size;     // Number of slots currently allocated in array
  ULONG numAtoms; // Number of actual atoms stored in list
  Atom *atoms;    // Array of atoms
} AtomList;


// Function prototypes

void BuildHeader (char *, char *);
void RecordAtom (AtomList *, char *);
void WriteAtomVertices (AtomList *, FILE *);
ULONG WriteBondEdges (AtomList *, FILE *);
void GetSubString (char *, char *, int, int);
float GetFloat (char *, int, int);
AtomList *AllocateAtomList (void);
void FreeAtomList (AtomList *);


int main (int argc, char **argv)
{
  FILE *inputFile;
  FILE *outputFile;
  char lineStr[LINELEN];
  char *readPtr;
  char header[LINELEN];
  BOOLEAN done;
  int i;
  float x, y, z;
  ULONG numEdges;
  AtomList *atomList;

  // Check command-line format
  if (argc != 3) {
    fprintf (stderr, "usage: %s <PDB file> <output file>\n", argv[0]);
    exit (1);
  }
  inputFile = fopen (argv[1], "r");
  if (inputFile == NULL) {
    fprintf (stderr, "unable to open PDB input file %s\n", argv[1]);
    exit (1);
  }
  outputFile = fopen (argv[2], "w");
  if (outputFile == NULL) {
    fprintf (stderr, "unable to open output file %s\n", argv[2]);
    fclose (inputFile);
    exit (1);
  }

  // Read input PDB file
  atomList = AllocateAtomList ();
  done = FALSE;
  readPtr = fgets (lineStr, LINELEN, inputFile);
  while ((! done) && (readPtr != NULL)) {
    if (strncmp (lineStr, HEADER_STR, strlen (HEADER_STR)) == 0)
      BuildHeader (header, lineStr);
    else if (strncmp (lineStr, ATOM_STR, strlen (ATOM_STR)) == 0)
      RecordAtom (atomList, lineStr);
    else if (strncmp (lineStr, ENDMDL_STR, strlen (ENDMDL_STR)) == 0)
      done = TRUE;
    else if (strncmp (lineStr, END_STR, strlen (END_STR)) == 0)
      done = TRUE;
    readPtr = fgets (lineStr, LINELEN, inputFile);
  }
  fclose (inputFile);
  fprintf (stdout, "Read %lu atoms from %s\n", atomList->numAtoms, argv[1]);

  // Write Subdue graph output file
  fprintf (outputFile, "%% Subdue graph of PDB file: %s\n", argv[1]);
  fprintf (outputFile, "%s\n", header);
  WriteAtomVertices (atomList, outputFile);
  fprintf (stdout, "Wrote %lu atom vertices to %s\n", atomList->numAtoms,
	   argv[2]);
  numEdges = WriteBondEdges (atomList, outputFile);
  fprintf (stdout, "Wrote %lu bond edges to %s\n", numEdges, argv[2]);

  FreeAtomList (atomList);
  fclose (outputFile);
  return 0;
}

void BuildHeader (char *header, char *lineStr)
     // Creates a Subdue comment containing the HEADER line from the PDB
     // file.
{
  int i;

  header[0] = COMMENT;
  header[1] = SPACE;
  for (i = 10; i < 66; i++) // PDB HEADER data in columns 11-66
    header[i-8] = lineStr[i];
  header[58] = END_STRING;
}

void RecordAtom (AtomList *atomList, char *lineStr)
{
  ULONG i;

  // first check if enough space in atomList
  if (atomList->size == atomList->numAtoms) {
    atomList->size = atomList->size + ATOMLIST_SIZE_INC;
    atomList->atoms = (Atom *) realloc (atomList->atoms,
					atomList->size * sizeof (Atom));
    if (atomList->atoms == NULL) {
      fprintf (stderr, "unable to reallocate atoms array\n");
      exit (1);
    }
  }
  // record new atom
  i = atomList->numAtoms;
  GetSubString(atomList->atoms[i].element, lineStr,
	       ELEMENT_START, ELEMENT_END);
  atomList->atoms[i].x = GetFloat (lineStr, X_START, X_END);
  atomList->atoms[i].y = GetFloat (lineStr, Y_START, Y_END);
  atomList->atoms[i].z = GetFloat (lineStr, Z_START, Z_END);
  atomList->numAtoms++;
}

void WriteAtomVertices (AtomList *atomList, FILE *outputFile)
     // Write PDB atoms as vertices in the Subdue graph file.
{
  ULONG i;

  for (i = 0; i < atomList->numAtoms; i++)
    fprintf (outputFile, "v %lu %s %% %8.3f %8.3f %8.3f\n",
	     (i + 1), atomList->atoms[i].element, atomList->atoms[i].x,
	     atomList->atoms[i].y, atomList->atoms[i].z);
}

ULONG WriteBondEdges (AtomList *atomList, FILE *outputFile)
     // Write an edge to the Subdue graph file for each bond in the PDB
     // file.  A bond is assumed between any two atoms whose distance is
     // between 0.4 and 1.9 angstroms (or between 0.4 and 1.2 angstroms if
     // one or both of the atoms are hydrogen.  Returns number of edges
     // written.
{
  ULONG numEdges;
  ULONG i, j;
  float distance;
  ULONG numAtoms;
  BOOLEAN bondExists;

  numEdges = 0;
  numAtoms = atomList->numAtoms;
  for (i = 0; i < numAtoms; i++) {
    for (j = i + 1; j < numAtoms; j++) {
      distance = 0.0;
      distance = distance +
	pow((atomList->atoms[i].x - atomList->atoms[j].x), 2.0);
      distance = distance +
	pow((atomList->atoms[i].y - atomList->atoms[j].y), 2.0);
      distance = distance +
	pow((atomList->atoms[i].z - atomList->atoms[j].z), 2.0);
      distance = sqrt (distance);
      bondExists = FALSE;
      if ((strcmp (atomList->atoms[i].element, "H") == 0) ||
	  (strcmp (atomList->atoms[j].element, "H") == 0)) {
	if ((distance >= BOND_DIST_HYDROGEN_MIN) &&
	    (distance <= BOND_DIST_HYDROGEN_MAX))
	  bondExists = TRUE;
      } else {
	if ((distance >= BOND_DIST_MIN) && (distance <= BOND_DIST_MAX))
	  bondExists = TRUE;
      }
      if (bondExists) {
	fprintf (outputFile, "u %lu %lu bond\n", (i + 1), (j + 1));
	numEdges++;
      }
    }
  }

  return numEdges;
}

void GetSubString (char *toStr, char *fromStr, int start, int end)
     // Stores the string in fromStr[start...end], without spaces, into
     // toStr.
{
  int i, j;

  j = 0;
  for (i = start - 1; i < end; i++)
    if (fromStr[i] != SPACE) {
      toStr[j] = fromStr[i];
      j++;
    }
  toStr[j] = END_STRING;
}

float GetFloat (char *lineStr, int start, int end)
     // Returns a float parsed from lineStr[start...end].
{
  int i, j;
  char floatStr[LINELEN];
  float x = 0.0;

  j = 0;
  for (i = start - 1; i < end; i++) {
    floatStr[j] = lineStr[i];
    j++;
  }
  floatStr[j] = END_STRING;
  i = sscanf (floatStr, "%f", &x);
  if (i == 0)
    fprintf (stderr, "unable to convert float from %s\n", floatStr);

  return x;
}

AtomList *AllocateAtomList (void)
     // Allocate structure to store list of atoms in PDB file.  The array
     // is initially allocated to length ATOMLIST_SIZE_INC.
{
  AtomList *atomList;

  atomList = (AtomList *) malloc (sizeof (AtomList));
  if (atomList == NULL) {
    fprintf (stderr, "unable to allocate atomList\n");
    exit (1);
  }
  atomList->size = ATOMLIST_SIZE_INC;
  atomList->numAtoms = 0;
  atomList->atoms = (Atom *) malloc (ATOMLIST_SIZE_INC * sizeof (Atom));
  if (atomList->atoms == NULL) {
    fprintf (stderr, "unable to allocate initial atoms array\n");
    exit (1);
  }
  return atomList;
}

void FreeAtomList (AtomList *atomList)
{
  free (atomList->atoms);
  free (atomList);
}
