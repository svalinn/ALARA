/* $Id: ADJLib.h,v 1.11 2003-06-10 20:23:11 wilsonp Exp $ */
#include "alara.h"

/** \brief Support for "adjoint" library format for reverse calculations

This class provides access to data libraries which follow the ALARA v1
binary merged library format.  This class is derived directly and
publicly from class DataLib.

*** Supported Data Library Formats ***

This catalogue of data library types should be included in every new
module developed to support a new data library format.

                       Input
Identifier     Value   String   Description
-------------------------------------------------------------------
DATALIB_NULL     0     null     A basic DataLib object
                                (should rarely be used in final object)
DATALIB_ALARA    1     alara    The default ALARA v1 binary format.
DATALIB_ASCII    2     ascii    A basic ASCII DataLib object
                                (should rarely be used in final object)
DATALIB_EAF      3     eaf      A data library following the formatting
                                definition of the EAF library (roughly
                                ENDF/B-6) 
DATALIB_ADJOINT  4     adj      An alara binary library in reversed format
                                for reverse calculations.
DATALIB_GAMMA    5     gamma    An alara binary library containing gamma
                                source information.
DATALIB_IEAF     6     ieaf     A data library with cross-section
                                libraries following the GENDF format and 
                                decay/gamma libraries following the 
                                formatting definition of the EAF library 
                                (roughly ENDF/B-6)
DATALIB_FEIND    7     feind     A FEIND library
DATALIB_ALARAJOY 8     ajoy     A data library following the formatting
                                definition of the FENDL 3.2b (TENDL 2017)
                                library (TENDL and PENDF format, converted
                                to GENDF format by an NJOY wrapped Python 
                                preprocessor).    
-------------------------------------------------------------------


*/

#ifndef ADJLIB_H
#define ADJLIB_H

#include "ALARALib.h"


class ADJLib : public ALARALib
{
protected:

  /// This class is only used as linked list structure internal to 
  /// the ADJLib class for performing reverse calculations.
  /** Each item in this list describes a reaction product.  The description
      is nothing more than a the isotope ID of the daughter and a linked list
      of reactions parents and library offsets that create this daughter. */
  class DaugItem
    {
    protected:

      /// This class is only used as a linked list structure internal
      /// to the ADJLib::DaugItem class for reverse calculations.
      /** Each item in this list describes a reaction that produces this
	  daughter isotope.  The description includes the ID of the parent
	  isotope and the binary library offset of the reaction in the
	  forward library */
      class ParItem
	{
	protected:
	  /// Isotope ID of parent
	  int kza;

	  /// Binary library offset in forward library for this reaction
	  long offset;
	  
	  /// Pointer to next item in linked list
	  ParItem* next;

	  /// Internal constructor used to insert list items
	  ParItem(const ParItem* cpyPtr);

	public:
	  /// Primary constructor
	  ParItem(const int parKza, const long rxnOffset);

	  /// Used to add/insert another item into the linked list
	  void add(const int parKza, const long rxnOffset);

	  /// Return the number of items in this linked list
	  int count();

	  /// Inline function to return a pointer to the next item in the list
	  ParItem* advance();

	  /// Inline function to return ID of a given list item
	  int getKza();

	  /// Inline function to return the offset of a given list item
	  long getOffset();
	};

      /// Pointer to head of list of parents/reactions
      ParItem* parList;

      /// Pointer to current parent/reaction
      ParItem* current;
      
      /// ID of daughter isotope
      int kza;

      /// Pointer to next item in linked list
      DaugItem *next;

      /// Internal constructor used to insert list items
      DaugItem(DaugItem* cpyPtr);

    public:
      /// Standard constuctor
      DaugItem(const int addKza, const int parKza, const long rxnOffset);

      /// Used to add/insert daughter or add to existing parent list
      void add(const int addKza, const int parKza, const long rxnOffset);

      /// Return the number of items in this linked list
      int count();

      /// Get the parent ID and binary library offset of the next reaction
      long getNextReaction(int& parKza);

      /// Inline function to return next list item
      inline DaugItem* advance();

      /// Inline function to return daughter isotope ID
      inline int getKza();

      /// Inline function to return number of items in parent list
      inline int countRxns();
    };
  
  /// Main list of daughters in this ADJLib
  DaugItem* daugList;
  
  /// Standard C file pointer to binary library
  FILE* normBinLib;

  /// Pointer to array with total production cross-section from all reactions
  float *totalXSection;

  /// Average decay energies for alpha, beta, & gamma
  float E[3];

  /// Half-life in seconds
  float thalf;

  /// Pointer to array with cross-section data
  float *xSection;

  /// String representation of particles emitted from reaction
  char emitted[32];

  /// copy the header information from the forward binary library 
  void copyHead();

  ///
  void getForwardData(int);

  ///
  void writeData(DaugItem*);

  ///
  void build();
  
public:
  ADJLib(char*,int setType=DATALIB_ADJOINT);
  ADJLib(char*,char*);

  
};



inline
ADJLib::DaugItem* ADJLib::DaugItem::advance()
{
  return next;
}

inline
int ADJLib::DaugItem::getKza()
{
  return kza;
}

inline
int ADJLib::DaugItem::countRxns()
{
  return parList->count();
}

/// Inline function to return a pointer to the next item in the list
inline
ADJLib::DaugItem::ParItem* ADJLib::DaugItem::ParItem::advance()
{
  return next;
}

/// Inline function to return ID of a given list item
inline
int ADJLib::DaugItem::ParItem::getKza()
{
  return kza;
}

/// Inline function to return the offset of a given list item
inline
long ADJLib::DaugItem::ParItem::getOffset()
{
  return offset;
}


  
#endif


