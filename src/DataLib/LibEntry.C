#include "DataLib.h"

DataLib::LibEntry::LibEntry()
{
  dataType = 0;
  format = 0;
  fname[0] = '\0';

  next = NULL;
}

DataLib::LibEntry::LibEntry(char* dataTypeStr, char* formatTypeStr, 
			    char* filename)
{
  dataType = convertLibType(dataTypeStr);
  format = convertLibType(formatTypeStr);
  strcpy(fname,filename);

  next = NULL;
}


DataLib::LibEntry* DataLib::LibEntry::getLibEntry(char* dataTypeStr,
						  istream& input)
{
  char formatTypeStr[16],filename[256];
  input >> formatTypeStr >> filename;
  next = new LibEntry(dataTypeStr,formatTypeStr,filename);

  return next;
}


