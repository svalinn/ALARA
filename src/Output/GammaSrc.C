#include "GammaSrc.h"

GammaSrc::GammaSrc(char *setName)
{
  nGrps = 0;
  name = NULL;
  if (setName != NULL)
    {
      name = new char[strlen(setName)+1];
      strcpy(name,setName);
    }

  next = NULL;
}

GammaSrc* GammaSrc::getGammaSrc(istream &input)
{
  int grpNum,maxNGrps = 16;
  double *tmp;
  char token[64];

  clearComment(input);
  input >> token;

  /* add new gamma source object */
  next = new GammaSrc(token);

  verbose(2,"Added gamma source structure %s",token);

  GammaSrc* gsrcPtr = next;

  /* start group boundary array */
  gsrcPtr->grpBnds = new double[maxNGrps];
  gsrcPtr->nGrps = 0;

  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      /* read group boundary */
      gsrcPtr->grpBnds[gsrcPtr->nGrps++] = atof(token);

      /* grow array as required */
      if (gsrcPtr->nGrps == maxNGrps)
	{
	  
	  maxNGrps *= 2;
	  tmp = gsrcPtr->grpBnds;
	  gsrcPtr->grpBnds = new double[maxNGrps];
	  for (grpNum=0;grpNum<gsrcPtr->nGrps;grpNum++)
	    gsrcPtr->grpBnds[grpNum] = tmp[grpNum];
	  delete tmp;
	}

      clearComment(input);
      input >> token;

    }

  /* shrink array to fit */
  if (gsrcPtr->nGrps>0)
    {
      tmp = gsrcPtr->grpBnds;
      gsrcPtr->grpBnds = new double[gsrcPtr->nGrps];
      for (grpNum=0;grpNum<gsrcPtr->nGrps;grpNum++)
	gsrcPtr->grpBnds[grpNum] = tmp[grpNum];
      delete tmp;
    }

  /* add extra group */
  gsrcPtr->nGrps++;

  /* return pointer to new object */
  return gsrcPtr;

}

/* find a named mixture in the list */
GammaSrc* GammaSrc::find(char* srchName)
{

  GammaSrc *ptr = this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (!strcmp(ptr->name,srchName))
	return ptr;
    }

  return NULL;
}
