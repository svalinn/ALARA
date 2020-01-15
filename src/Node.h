/* $Id: Node.h,v 1.15 2003-01-13 04:34:51 fateneja Exp $ */

/*
  lambdaCache, heatCache, alphaCache, 
 betaCache, gammaCache, wdrCache : DataCache
   Using this cache system, the decay data used in post-processing
   only needs to be read from the library once.
*/

#include "alara.h"

#ifndef NODE_H
#define NODE_H

#include "NuclearData.h"
#include "TreeInfo.h"
#include "Statistics.h"

#define WDR_KZAMODE 0
#define WDR_SYMMODE 1

/** \brief This class stores the information about a particular node in a chain.
 *
 *  It is derived publicly from classes NuclearData and TreeInfo.  As a
 *  chain is created, objects of class Node are added to the bottom of the
 *  chain to represent the isotopes in the chain.  Following truncation,
 *  the objects are removed from the bottom of a chain.  Through its base
 *  classes, it contains all the information about the isotope itself.
 */

class Node : public NuclearData, public TreeInfo
{
protected:

  /// This is the integer representation of the atomic number, the atomic
  /// mass number and the isomeric number.  
  /** KZA = (Z*1000+A)*10+M */
  int kza;
  
  /// This is an interger that stores the global idenifier for this node.
  /** It is returned by Statistics::accountNode.  It is used to identifying
      parents in the binary tree output. */
  int nodenum;

  static DataCache lambdaCache;
  static DataCache heatCache;
  static DataCache alphaCache;
  static DataCache betaCache;
  static DataCache gammaCache;
  static DataCache wdrCache;


  /// This function searches back up the chain and find the first
  /// occurrence of an isotope with the same KZA as the current one.  
  /** The rank of this isotope is returned for use in Chain::loopRank[].*/
  int findLoop();


public:
  /// The default constructor when called with no arguments
  Node(char* isoName=NULL);
  
  /// This constructor invokes an initialization list.
  Node(int,Node*,double*,int,int);
  
  /// This function passes the 'kza' value and 'this' pointer to the
  /// readData() function of the 'dataLib' member object of base class
  /// NuclearData.  
  void readData();

  /// This function points the appropriate elements of the
  /// 'Chain::rates' array, passed in argument 1, at the rate vectors
  /// and data for this isotope.
  void copyRates(double**,const int, int*);

  /// This function re-initializes the elements of the 'rates' array.
  void delRates(double**,const int, int*);

  /// This function combines the current state with the information
  /// passed in the argument to determine a new state.
  int stateEngine(int stateBits=-1);
  
  /// This function adds the next Node (either daughter or parent
  /// depending on direction of solution) for the isotope in question.
  Node* addNext(int&);

  /// This function deletes the "next" node (and thus the entire
  /// sub-chain) of the current node.
  void prune();

  /// This function returns a pointer to the "previous" node of the
  /// current isotope.
  Node* retract();

  /// Inline function to return boolean indicating whether or not this
  /// is a new node in the chain by comparing nRxn to <0.
  int newNode() {  return (nPaths<0); }

  /// Inline function provides access to member 'kza'.
  int getKza() { return kza; };

  /// Function searches through a chain for a given rank and returns 
  /// the kza of that rank.
  int getRankKza(int);

  /// This function is used to determine the indexing information for
  /// the RateCache concept.
  void getRxnInfo(double*,int&,int&,int&);

  /// Inline function that sends information about the current node to the
  /// statistics routines for diagnostic and tree output.
  int count(double* relProd)
   {  
     if (prev)
       {
	 nodenum= Statistics::accountNode(kza,prev->emitted[prev->pathNum-1],
				       rank,state,relProd, prev->nodenum);
	 return nodenum;
       }   
      else
	{
	  nodenum=Statistics::accountNode(kza,NULL,
				       rank,state,relProd,0);
	  return nodenum;
	}
    };

  /// This function accesses the data library to get the decay constant
  /// for the isotope indicated by the argument, first checking in the
  /// cache.
  double getLambda(int);

  /// This function accesses the data library to get the total decay
  /// heat for the isotope indicated by the argument, first checking in
  /// the cache.
  double getHeat(int);

  /// This function accesses the data library to get the alpha decay
  /// heat for the isotope indicated by the argument, first checking in
  /// the cache.
  double getAlpha(int);

  /// This function accesses the data library to get the beta decay heat
  /// for the isotope indicated by the argument, first checking in the
  /// cache.
  double getBeta(int);

  /// This function accesses the data library to get the gamma decay
  /// heat for the isotope indicated by the argument, first checking in
  /// the cache.
  double getGamma(int);

  double getWDR(int);

  /// Get charged particle cross sections
  double** getCPXS(int findKZA);

  /// This function opens the file whose name is given in the argument
  /// and reads the WDR thresholds into the wdrCache.
  static void loadWDR(char*);
};

#endif
