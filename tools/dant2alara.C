#include <iostream>
#include <stdio.h>

#define SLONG sizeof(long)
#define SINT sizeof(int)
#define SFLOAT sizeof(float)
#define SDOUBLE sizeof(double)

/* import std::istream, std::ofstream since alternative implmentations
 * are unlikely */
using std::istream;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::ios;


int main(int argc, char* argv[]) {

  char rtfluxFname[256];

  int f77_reclen; /// needed to accommodate strange F77 binary strucutre
  int readInt;
  float readFlt;
  char buffer[256];

  int grpLo, grpUp, grpHi;

  /* get rtflux filename */
  cout << "Enter the rtflux/atflux filename: ";
  cin >> rtfluxFname;

  /* read rtflux file into memory */
  // open file
  FILE* binFile = fopen(rtfluxFname,"rb");

  // read file header
  fread((char*)&f77_reclen,SINT,1,binFile);
  fread(buffer,1,24,binFile);
  fread((char*)&readInt,SINT,1,binFile);
  fread((char*)&f77_reclen,SINT,1,binFile);
  
  // read dimensions
  int ndim, ngrp, ninti, nintj, nintk, iter, nblok;
  float effk, power;
  fread((char*)&f77_reclen,SINT,1,binFile);
  fread((char*)&ndim, SINT,1,binFile);
  fread((char*)&ngrp, SINT,1,binFile);
  fread((char*)&ninti,SINT,1,binFile);
  fread((char*)&nintj,SINT,1,binFile);
  fread((char*)&nintk,SINT,1,binFile);
  fread((char*)&readInt, SINT,1,binFile);
  fread((char*)&readFlt, SFLOAT,1,binFile);
  fread((char*)&readFlt,SFLOAT,1,binFile);
  fread((char*)&nblok, SINT,1,binFile);
  fread((char*)&f77_reclen,SINT,1,binFile);

  // limit to 1-D
  if (ndim > 1)
    {
      cerr << "RFLUX file: " << rtfluxFname << " is 2- or 3-dimensional.  This feature currently only supports 1-D." << endl;
      exit(-1);
    }

  /// read blocks (1-D)
  double* fluxIn = new double[ninti*ngrp];
  for (int blkNum=0;blkNum<nblok;blkNum++)
    {
      grpLo =   blkNum   * ((ngrp-1)/nblok + 1);
      grpUp = (blkNum+1) * ((ngrp-1)/nblok + 1)-1;
      grpHi = std::min(ngrp,grpUp);
      fread((char*)&f77_reclen,SINT,1,binFile);
      fread((char*)(fluxIn+grpLo*ninti),SDOUBLE,(grpHi-grpLo+1)*ninti,binFile);
      fread((char*)&f77_reclen,SINT,1,binFile);
    }

  /* write flux info to file */
  for (int volNum=0;volNum<ninti;volNum++)
    {
      for (int gNum=0;gNum<ngrp;gNum++)
	{
	  cout << fluxIn[gNum*ninti+(volNum)];
	  if ( gNum%6 )
	    cout << "  ";
	  else
	    cout << endl;
	}
      cout << endl << endl;
    }

  delete fluxIn;


}
