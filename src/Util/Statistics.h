#include "alara.h"

#ifndef _STATISTICS_H
#define _STATISTICS_H

class Statistics 
{
protected:
  static ofstream treeFile;
  static int tree, nodeCtr;

public:
  
  static int accountNode(int,char*,int,int,double*);

  static void initTree(char*);
  static void closeTree();

};

#endif
