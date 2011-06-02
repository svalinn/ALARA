/* $Id: input.C,v 1.14 2007-12-20 22:03:56 wilsonp Exp $ */
#include "alara.h"
#include "input_tokens.h"
#include "dflt_datadir.h"
#include <sys/types.h>
#include <sys/stat.h>

const char *tokenList = "\
geometry     \
mixture      \
flux         \
pulsehistory \
schedule     \
dimension    \
minor_radius \
major_radius \
cooling      \
mat_loading  \
volumes      \
material_lib \
element_lib  \
data_library \
truncation   \
output       \
format_output\
spatial_norm \
convert_lib  \
dump_file    \
impurity     \
solve_zones  \
skip_zones   \
ignore       \
ref_flux_type\
cp_libs";


#define MAXLINELENGTH 256

istream* openFile(const char *fName)
{
  return new ifstream(fName);
}

/* function to clear all blank lines and comment lines 
 * from the input FILE stream */
void clearComment(istream& inFile)
{
  /* look at the next character in the stream */
  char charInput = inFile.peek();

  while (charInput != 'A')
    switch (charInput)
      {
      case '#' : /* comment */
	inFile.ignore(MAXLINELENGTH,'\n');
	charInput = inFile.peek();
	break;
      case ' ' : /* whitespace */
      case '\n': /* whitespace */
      case '\t': /* whitespace */
	inFile.get();
	charInput = inFile.peek();
	break;
      default:
	charInput = 'A';
      }
      
}


/* convert an input token to a numerical type */
int tokenType(char* token)
{
  int tokenWidth = 13;
  unsigned int idx;

  for (idx=0;idx<strlen(token);idx++)
    token[idx] = tolower(token[idx]);

  char *tokenType = strstr(tokenList,token);
  if (tokenType != NULL)
    {
      /* debug(1,"token match made %s <=> %s",token,tokenType); */
      return (tokenType-tokenList)/tokenWidth;
    }
  else
    return -1;
}

/* convert a time in given units to seconds */
double convertTime(double inTime, char units)
{
  switch (units)
    {
      /* no 'break' statements because of 
       * desire for cumulative effect */
    case 'c' : /* century */
      inTime *= 100;
    case 'y' : /* year */
      inTime *= 365/7;
    case 'w' : /* week */
      inTime *= 7;
    case 'd' : /* day */
      inTime *= 24;
    case 'h' : /* hour */
      inTime *= 60;
    case 'm' : /* minute */
      inTime *= 60;
    case 's' : /* second */
      break;
    }

  return inTime;
}



const char* searchNonXSPath(const char* filename)
{

  const char *pathVar = getenv("ALARA_DATADIR");
  const char *dfltPath = DFLT_DATADIR;

  return searchPath(filename,pathVar,dfltPath);


}


const char* searchXSPath(const char* filename)
{

  const char *pathVar = getenv("ALARA_XSDIR");
  const char *dfltPath = DFLT_XSDIR;

  return searchPath(filename,pathVar,dfltPath);

}

const char* searchPath(const char* filename, const char* envPathVar, const char* builtinPathVar)
{
  struct stat stat_info;
  int stat_result = -1;
  std::string searchFilename = "./";

  searchFilename += filename;

  verbose(100,"Looking for %s",searchFilename.c_str());

  if ( (stat_result = stat(searchFilename.c_str(),&stat_info)) != 0)
    {
      if (envPathVar)
	{
	  std::string envPath = envPathVar;
	  
	  std::string::size_type begin = 0;
	  std::string::size_type end = 0;
	  
	  if (envPath[envPath.length()-1] != ':')
	    envPath += ":";
	  
	  while (stat_result != 0 && begin < envPath.length())
	    {
	      end = envPath.find(":",begin);
	      std::string thisDir = envPath.substr(begin,end-begin);
	      if (thisDir[thisDir.length()-1] == '/')
		thisDir.erase(thisDir.length()-1,1);
	      searchFilename = thisDir + "/" + filename;

	      verbose(100,"Looking for %s",searchFilename.c_str());

	      stat_result = stat(searchFilename.c_str(),&stat_info);
	      begin = end + 1;
	    }
	}

      if (stat_result != 0)
	{
	  searchFilename = builtinPathVar;
	  searchFilename += "/";
	  searchFilename += filename;
	  verbose(100,"Looking for %s",searchFilename.c_str());
	  stat_result = stat(searchFilename.c_str(),&stat_info);
	}

    }
  
  if (stat_result != 0)
    searchFilename = filename;
  else
    verbose(100,"Found %s",searchFilename.c_str());
    

  return strdup(searchFilename.c_str());

}

