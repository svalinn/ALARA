#include "Statistics.h"

#include "Chains/truncate.h"

ofstream Statistics::treeFile;
int Statistics::tree = FALSE;
int Statistics::nodeCtr = 0;

void Statistics::initTree(char* fname)
{
  treeFile.open(fname);
  if (treeFile)
    tree = TRUE;
}

void Statistics::closeTree()
{
  if (tree)
    treeFile.close();
}

int Statistics::accountNode(int kza, char* emitted, int rank, int state, 
			     double* relProd)
{
  char isoSym[10];

  nodeCtr++;

  if (tree)
    {
      while (rank-->0)
	treeFile << "\t";

      if (emitted)
	treeFile << "-(" << emitted << ")-> ";
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
	  treeFile << " < " << endl;
	  break;
	}
    }

  return nodeCtr;
}

