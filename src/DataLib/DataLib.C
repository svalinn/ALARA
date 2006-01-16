/* $Id: DataLib.C,v 1.9 2006-01-16 19:11:51 phruksar Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Lib: functions directly related to library handling
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "DataLib.h"
#include <string>

using namespace std;

/* possible library sub-types */
#include "ASCIILib/EAFLib/EAFLib.h"
#include "ASCIILib/IEAFLib/IEAFLib.h"
#include "ALARALib/ALARALib.h"
#include "ALARALib/ADJLib.h"


const char *libTypes =  "\
null  \
alara \
ascii \
eaf   \
adj   \
gamma \
ieaf  \
feind ";

const int libTypeLength = 7;

const char *libTypeStr[] = {
  "an unspecified",
  "an 'alaralib' ALARA",
  "an unspecified ASCII",
  "an EAF library",
  "an 'adjlib' ALARA",
  "a 'gammalib' ALARA",
  "an IEAF library",
  "a FEIND library"};

const char *libTypeSuffix[] = {
  ".null",
  ".lib",
  ".null",
  ".eaf",
  ".lib",
  ".gam",
  ".ieaf",
  ".feind"};


/****************************
 ********* Service **********
 ***************************/

/** It takes the text string argument and converts it to an integer
    type, and then based on that type creates a new object of the
    correct derived type reading further information, as required,
    from the inpout file attached to the passed stream reference.  A
    pointer to the newly created object. */
DataLib* DataLib::newLib(char* libType, istream& input)
{
  DataLib *dl;
  int type = convertLibType(libType);
  debug(4,"Data lib is type %d.",type);

  switch (type)
    {
    case DATALIB_EAF:
    case DATALIB_IEAF:
      convertLib(libType,DATALIB_ALARA,input);
      dl = new ALARALib(ALARAFNAME);
      break;
    case DATALIB_ALARA:
      char alaraLibName[256];
      input >> alaraLibName;
      dl = new ALARALib(alaraLibName,type);
      verbose(3,"Openned binary library with %d parents and %d groups.",
	      dl->nParents,dl->nGroups);
      break;
    case DATALIB_FEIND:
      vector<string> libArg(3);
      input >> libArg[0] >> libArg[1] >> libArg[2];
      dl = new FEINDLib(libArg, type);
      verbose(3,"Openned FEIND library with %d parents and %d groups.",
	      dl->nParents,dl->nGroups);      
      break;
    case DATALIB_ADJOINT:
      char adjointLibName[256];
      input >> adjointLibName;
      dl = new ADJLib(adjointLibName,type);
      verbose(3,"Openned adjoint library with %d parents and %d groups.",
	      dl->nParents,dl->nGroups);
      break;
    case DATALIB_GAMMA:
      char gammaLibName[256];
      input >> gammaLibName;
      dl = new ALARALib(gammaLibName,type);
      verbose(3,"Openned binary gamma library.");
      break;
    default:
      error(1000,"Data library type %s (%d) is not yet supported.",
	    libType, type);
    }

  return dl;
}

void DataLib::convertLib(char *fromTypeStr, int toType, istream& input)
{
  DataLib *dl;

  int fromType = convertLibType(fromTypeStr);
  char transFname[256], decayFname[256];

  switch (fromType*100+toType)
    {
    case EAF2ALARA:
      input >> transFname >> decayFname;
      verbose(3,"Openning EAF formatted libraries %s, %s for conversion",
	      transFname,decayFname);
      dl = new EAFLib(transFname,decayFname,ALARAFNAME);
      delete dl;
      break;
    case IEAF2ALARA:
      input >> transFname >> decayFname;
      verbose(3,"Openning IEAF formatted libraries %s, %s for conversion",
	      transFname,decayFname);
      dl = new IEAFLib(transFname,decayFname,ALARAFNAME);
      delete dl;
      break;
    default:
      error(1001,"Conversion from %s (%d) to (%d) is not yet supported.",
	    fromTypeStr, fromType, toType);
    }

}

void DataLib::convertLib(istream& input)
{
  DataLib *dl;
  char fromTypeStr[16], toTypeStr[16];
  int fromType, toType;
  char alaraFname[256];
  char transFname[256], decayFname[256];

  input >> fromTypeStr >> toTypeStr;
  fromType = convertLibType(fromTypeStr);
  toType = convertLibType(toTypeStr);
  debug(4,"Data lib is type %d.",fromType);

  switch (fromType*100+toType)
    {
    case EAF2ALARA:
      input >> transFname >> decayFname >> alaraFname;
      verbose(3,"Openning EAF formatted libraries %s, %s for conversion into ALARA library %s",
	      transFname,decayFname,alaraFname);
      dl = new EAFLib(transFname,decayFname,alaraFname);
      delete dl;
      verbose(3,"Converted libraries with %d parents and %d groups.",
	      dl->nParents,dl->nGroups);
      break;
    case IEAF2ALARA:
      input >> transFname >> decayFname >> alaraFname;
      verbose(3,"Openning IEAF formatted libraries %s, %s for conversion into ALARA library %s",
	      transFname,decayFname,alaraFname);
      dl = new IEAFLib(transFname,decayFname,alaraFname);
      delete dl;
      verbose(3,"Converted libraries with %d parents and %d groups.",
	      dl->nParents,dl->nGroups);
      break;
    case ALARA2ADJ:
      char adjointLibName[256];
      input >> alaraFname >> adjointLibName;
      dl = new ADJLib(alaraFname,adjointLibName);
      delete dl;
      break;
    default:
      error(1001,"Conversion from %s (%d) to %s (%d) is not yet supported.",
	    fromTypeStr, fromType, toTypeStr, toType);
    }

}

/****************************
 ********* Utility **********
 ***************************/

int DataLib::convertLibType(char* libType)
{
  char *strPtr = libType;

  while (*strPtr != '\0')
    {
      *strPtr = tolower(*strPtr);
      strPtr++;
    }

  strPtr = strstr(libType,"lib");
  *strPtr = '\0';

  int type = (strstr(libTypes,libType)-libTypes)/libTypeLength;

  return type;

}

/****************************
 ********* Virtual **********
 ***************************/

/** This must be redefined in the derived classes.  The arguments are
    intended to be the KZA number of an isotope in question and a
    pointer to the NuclearData object which will store the data being
    read. */
void DataLib::readData(int kza, NuclearData* data)
{
  error(9000, "Programming error: DataLib::readData() must be called from a derived object.");
}


void DataLib::readGammaData(int kza, GammaSrc* gammaSrc)
{
  error(9000, "Programming error: DataLib::readGammaData() must be called from a derived object.");
}
