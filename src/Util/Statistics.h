/* $Id: Statistics.h,v 1.10 2001-07-23 20:02:22 wilsonp Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is used to measure some statistics of each run.

 *** Static Members ***

 treeFile : ofstream
    This file is used to record the tree information created during
    the chain building process.

 binFile : FILE*
    This is a pointer to the file that will used to record the tree 
    information created during the chain building process in a binary form.

 tree : int 
    This flag indicates whether or not a tree file has been requested
    for this run.

 treebin : int
    This flag indicates whether or not a binary tree file has been opened.

 nodeCtr : int
    This is a counter for the number of nodes encountered during the
    solution of this problem.

 chainCtr : int
    This is a counter for the number of chains encountered during the
    solution of this problem.

 maxRootRank : int
    This keeps track of the maximum rank attained in any chain during
    the solution of the current root.  It is reset for each root.

 maxProblemRank : int
    This keeps track of the maximum rank attained in any chain during
    the solution of the entire problem.

 ticks : float
    This is a machine-dependent normalization used to convert the
    cputime measures to seconds.

 runtime : float[2]
    This variable stores a pair or times, used to find delta times for
    different parts of the solution.

 *** Static Member Functions ***


 int accountNode(int,char*,int,int,double*, int)
    This function increments nodeCtr, and then writes the information
    about this node to the tree file(s), if requested. The current value
    of nodeCtr (after the incrementing) is returned.

 int accountChain(int)
    This inline function increments chainCtr and, if the rank given in
    the argument is greater than 'maxRootRank', it updates
    'maxRootRank'.  The current value of chainCtr (after the
    incrementing) is returned.

 int accountMaxRank()
    This inline function is called at the end of the solution for a
    root isotope.  It first updates maxProblemRank if maxRootRank is
    higher than maxProblemRank, resets maxRootRank to 0, and returns
    the value of maxRootRank upon entry.

 void initTree(char*)
    This function simply opens the 'treeFile' ofstream with the
    filename given in the first argument.

 void closeTree()
    This function simply closes the 'treeFile' ofstream.


 FILE* openBinFile(fname*)
    This function opens the binaray file specified by fname using the file
    pinter binFile


 void cpuTime(float&,float&)
    This function finds the current runtime from a system call, and
    then returns the time since the last call to cpuTime in the first
    arguement, and the total cputime in the second argument.

 int numNodes()
    This inline function provides access to the current value of
    nodeCtr.

 int maxRank()
    This inline function provides access to the current value of
    maxProblemRank.

 int getNodeCtr()
    This function simply returns the value of NodeCtr

*/

#ifndef _STATISTICS_H
#define _STATISTICS_H

class Statistics 
{
protected:
  static ofstream treeFile;
  static FILE* binFile;
  static int tree, treebin, nodeCtr, chainCtr, maxRootRank, maxProblemRank;
  static float ticks;
  static float runtime[2];

public:
  
  static int accountNode(int,char*,int,int,double*, int);
  static int accountChain(int rank)
    { chainCtr++; maxRootRank=max(maxRootRank,rank); return chainCtr;};
  static int accountMaxRank()
  {
    int tmp = maxRootRank;
    maxProblemRank = max(maxRootRank,maxProblemRank);
    maxRootRank = 0;
    return tmp;
  };

  static int getNodeCtr() {return nodeCtr;};
  static void initTree(char*);
  static void closeTree();
  static FILE* openBinFile(char *);
  static void cputime(float&,float&);

  

  static int numNodes()
    {return nodeCtr;};
  static int maxRank()
    { return maxProblemRank; };

};

#endif
