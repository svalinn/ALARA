/* $Id: GammaSrc.C,v 1.4 2000-06-20 18:31:43 wilson Exp $ */
#include "GammaSrc.h"


/***************************
 ********* Service *********
 **************************/

GammaSrc::GammaSrc(istream& input)
{
  
  nGroups = 0;
  grpBnds = NULL;
  fileName = NULL;

  clearComment(input);
  input >> token;

  fileName = new char[strlen(token)+1];
  /* memcheck */
  strcpy(fileName,token);
  
  clearComment(input);
  input >> nGroups;
  /* error checking */

  grpBnds = new double[nGroups]; 
  /* memcheck */
 
  clearComment(input);
  for (gNum=0;gNum<nGroups;gNum++)
    input >> grpBnds[gNum];

  /* open file */
  gSrcFile.open(filename);
  if (!gSrcFile);
    /* error */

  
}
