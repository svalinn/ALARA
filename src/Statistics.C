/* $Id: Statistics.C,v 1.10 2003-01-13 04:35:01 fateneja Exp $ */
#include "Statistics.h"

#include <unistd.h>
#include <sys/times.h>
#include<stdio.h>

#include "Chains/truncate.h"

ofstream Statistics::treeFile;
FILE * Statistics::binFile;
int Statistics::tree = FALSE;
int Statistics::treebin = FALSE;
int Statistics::nodeCtr = 0;
int Statistics::chainCtr = 0;
int Statistics::maxRootRank = 0;
int Statistics::maxProblemRank = 0;
float Statistics::ticks = (float)sysconf(_SC_CLK_TCK);
float Statistics::runtime[2] = { 0, 0 };


void Statistics::initTree(char* fname)
{
  
  treeFile.open(fname);
  if (treeFile)
    tree = TRUE;

  char ext[]=".bin";
  char binName[256];
  strcpy(binName, fname);
  fname=strtok(binName, ".");
  strncat(fname, ext, 4);
  
 if( openBinFile(fname))
   treebin=TRUE;
}



FILE * Statistics::openBinFile(char* fname)
{
  binFile=fopen(fname, "wb");
  if (binFile==NULL)
    error(1500,"Error opening binary tree file: %s\n", fname);

  return binFile;
}


void Statistics::closeTree()
{
  if (tree)
    treeFile.close();

  if (treebin)
    fclose(binFile);
    
}

/** The current value of nodeCtr (after the incrementing) is returned. */
int Statistics::accountNode(int kza, char* emitted, int rank, int state, 
			     double* relProd, int parentnum)
{
  float newRelProd; 
  char isoSym[10];
  nodeCtr++;

  if (tree)
    {
      while (rank-->1)
	treeFile << "\t|";
      if (rank > -1)
	treeFile << "\t";

      if (emitted)
	treeFile << "|-(" << emitted << ")-> ";
      treeFile << isoName(kza,isoSym);
      if (relProd != NULL)
	treeFile << " (" << relProd[0] << ")";
      else 
	treeFile << " ( - ) ";

      switch(state)
	{
	case CONTINUE:
	  treeFile << " -" << endl;
	  break;
	case TRUNCATE:
	  treeFile << " /" << endl;
	  break;
	case TRUNCATE_STABLE:
	  treeFile << " *" << endl;
	  break;
	case IGNORE:
	  treeFile << " <" << endl;
	  break;
	}
    }


  if(treebin)
    {
      
      int itemsWritten=fwrite(&parentnum, sizeof(int), 1, binFile);
      //check to see if anything was actually written
      if( itemsWritten!=1)
	error(1501,"There was an error in writng to the binary tree file\n");
      
      itemsWritten=fwrite(&nodeCtr, sizeof(int), 1, binFile);
      //check to see if anything was actually written
      if( itemsWritten!=1)
	error(1501,"There was an error in writng to the binary tree file\n");

      itemsWritten=fwrite(&kza, sizeof(int), 1, binFile);
      //check to see if anything was actually written
      if( itemsWritten!=1)
	error(1501,"There was an error in writng to the binary tree file\n");
       
      if (relProd != NULL)
	newRelProd = relProd[0];
      else
	newRelProd = -1;
	
      itemsWritten=fwrite(&newRelProd, sizeof(float), 1, binFile);
      //check to see if anything was actually written
      if( itemsWritten!=1)
	error(1501,"There was an error in writng to the binary tree file\n");
    }

  return nodeCtr;
 
}

/** The current value of nodeCtr (after the incrementing) is returned. */
void Statistics::cputime(float &increment, float &total)
{
  static struct tms time0;

  times(&time0);

  runtime[1] = runtime[0];
  runtime[0] = (float)time0.tms_utime/ticks;

  total = runtime[0];
  increment = total-runtime[1];

}
















