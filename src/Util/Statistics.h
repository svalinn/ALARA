/* $Id: Statistics.h,v 1.7 1999-08-24 23:19:43 wilson Exp $ */
#include "alara.h"

/* #define max1(x,y) (x>y?x:y) */


#ifndef _STATISTICS_H
#define _STATISTICS_H

class Statistics 
{
protected:
  static ofstream treeFile;
  static int tree, nodeCtr, chainCtr, maxRootRank, maxProblemRank;
  static float ticks;
  static float runtime[2];

public:
  
  static int accountNode(int,char*,int,int,double*);
  static int accountChain(int rank)
    { chainCtr++; maxRootRank=max(maxRootRank,rank); return chainCtr;};
  static int accountMaxRank()
  {
    int tmp = maxRootRank;
    maxProblemRank = max(maxRootRank,maxProblemRank);
    maxRootRank = 0;
    return tmp;
  };

  static void initTree(char*);
  static void closeTree();
  static void cputime(float&,float&);

  static int numNodes()
    { return nodeCtr; };
  static int maxRank()
    { return maxProblemRank; };

};

#endif
