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
      char transFname[256], decayFname[256];
      input >> transFname >> decayFname;
      verbose(3,"Openning EAF formatted libraries %s, %s for conversion",
	    transFname,decayFname);
      dl = new EAFLib(transFname,decayFname);
      delete dl;
      dl = new ALARALib();
      verbose(3,"Converted libraries and openned binary library with %d parents and %d groups.",
	      dl->nParents,dl->nGroups);
      break;
    case DATALIB_ALARA:
      char alaraLibName[256];
      input >> alaraLibName;
      dl = new ALARALib(alaraLibName);
      verbose(3,"Openned binary library with %d parents and %d groups.",dl->nParents,dl->nGroups);
      break;
    default:
      error(160,"Data library type %s (%d) is not yet supported.",
	    libType, type);
    }

  return dl;
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

  char *libTypes =  "\
null  \
alara \
ascii \
eaf   ";

  int libLength = 6;
  int type = (strstr(libTypes,libType)-libTypes)/libLength;

  return type;

}

/****************************
 ********* Virtual **********
 ***************************/

void DataLib::readData(int kza, NuclearData* data)
{
  error(200, "Programming error: DataLib::readData() must be called from a derived object.");
}


