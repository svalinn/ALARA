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
  
  int firstGroup, lastGroup;

  /* get rtflux filename */
  cerr << "Enter the rtflux/atflux filename: ";
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

  int nint = ninti * nintj * nintk;

  // limit to 1-D
  if (ndim > 1)
    {
      cerr << "RFLUX file: " << rtfluxFname << " is 2- or 3-dimensional.  This feature currently only supports 1-D." << endl;
      exit(-1);
    }

  firstGroup = -1;
  lastGroup = -1;

  while (firstGroup <1 || firstGroup > ngrp) {
    /// get info from user
    cerr << "This file has " << ngrp << " groups in ("
	 << ninti << "x" << nintj << "x" << nintk << ")=" << nint
	 << "intervals and " << ndim << " dimension." << endl;
    cerr << "Enter the first group number to export: ";
    cin >> firstGroup;
    
    if (firstGroup <1 || firstGroup > ngrp)
      cerr << "That group is not in the correct range." << endl;
  }

  while (lastGroup < firstGroup || lastGroup > ngrp) {
    cerr << "Enter the number of groups to export, OR" << endl;
    cerr << "\tenter the last group as a negative number: ";
    cin >> lastGroup;

    if (lastGroup < 0) 
      lastGroup = -lastGroup;
    else
      lastGroup = firstGroup + lastGroup - 1;

    if (lastGroup < firstGroup || lastGroup > ngrp)
      cerr << "That group is not in the correct range." << endl;
  }

  cerr << "Exporting groups " << firstGroup << " through " 
       << lastGroup << " to stdout." << endl;
    
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


  cout.ios::precision(5);
  cout.ios::setf(ios::scientific);
  cout.ios::width(13);

  /* write flux info to file */
  for (int volNum=0;volNum<ninti;volNum++)
    {
      for (int gNum=firstGroup-1;gNum<lastGroup;gNum++)
	{
          cout << fluxIn[gNum*ninti+(volNum)];
	  if ( (gNum+1)%6 )
	    cout << "  ";
	  else
	    cout << endl << "  ";
	}
      cout << endl << endl << "  ";
    }

  delete fluxIn;


}
