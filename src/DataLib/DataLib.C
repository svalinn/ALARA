/* $Id: DataLib.C,v 1.6 2000-07-07 02:15:51 wilson Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Lib: functions directly related to library handling
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "DataLib.h"

/* possible library sub-types */
#include "ASCIILib/EAFLib/EAFLib.h"
#include "ALARALib/ALARALib.h"
#include "ALARALib/ADJLib.h"


const char *libTypes =  "\
null  \
alara \
ascii \
eaf   \
adj   \
gamma ";

const int libTypeLength = 6;

const char *libTypeStr[] = {
  "an unspecified",
  "an 'alaralib' ALARA",
  "an unspecified ASCII",
  "an EAF library",
  "an 'adjlib' ALARA",
  "a 'gammalib' ALARA"};

const char *libTypeSuffix[] = {
  ".null",
  ".lib",
  ".null",
  ".eaf",
  ".lib",
  ".gam"};


/****************************
 ********* Service **********
 ***************************/

DataLib* DataLib::newLib(char* libType, istream& input)
{
  DataLib *dl;
  int type = convertLibType(libType);
  debug(4,"Data lib is type %d.",type);

  switch (type)
    {
    case DATALIB_EAF:
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

  switch (fromType*100+toType)
    {
    case EAF2ALARA:
      char transFname[256], decayFname[256];
      input >> transFname >> decayFname;
      verbose(3,"Openning EAF formatted libraries %s, %s for conversion",
	      transFname,decayFname);
      dl = new EAFLib(transFname,decayFname,ALARAFNAME);
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

  input >> fromTypeStr >> toTypeStr;
  fromType = convertLibType(fromTypeStr);
  toType = convertLibType(toTypeStr);
  debug(4,"Data lib is type %d.",fromType);

  switch (fromType*100+toType)
    {
    case EAF2ALARA:
      char transFname[256], decayFname[256];
      input >> transFname >> decayFname >> alaraFname;
      verbose(3,"Openning EAF formatted libraries %s, %s for conversion into ALARA library %s",
	      transFname,decayFname,alaraFname);
      dl = new EAFLib(transFname,decayFname,alaraFname);
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

void DataLib::readData(int kza, NuclearData* data)
{
  error(9000, "Programming error: DataLib::readData() must be called from a derived object.");
}


void DataLib::readGammaData(int kza, GammaSrc* gammaSrc)
{
  error(9000, "Programming error: DataLib::readGammaData() must be called from a derived object.");
}
