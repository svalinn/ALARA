/* $Id: input.C,v 1.9 2000-06-20 01:50:51 wilson Exp $ */
#include "alara.h"
#include "input_tokens.h"


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
ref_flux_type";

#define MAXLINELENGTH 256

istream* openFile(char *fName)
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
    token[0] = tolower(token[0]);

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
