/* (Potential) File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */

#include "Flux.h"

#include "Volume.h"
#include "Calc/VolFlux.h"

/***************************
 ********* Service *********
 **************************/

Flux::Flux(int inFormat, char *flxName, char *fName, 
	   double inScale, int inSkip) :
  format(inFormat),  skip(inSkip), scale(inScale)
{
  fluxName = NULL;
  fileName = NULL;

  if (flxName != NULL)
    {
      fluxName = new char[strlen(flxName)+1];
      memCheck(fluxName,"Flux::Flux(...) constructor: fluxName");
      strcpy(fluxName,flxName);
    }

  if (fName != NULL)
    {
      fileName = new char[strlen(fName)+1];
      memCheck(fileName,"Flux::Flux(...) constructor: fileName");
      strcpy(fileName,fName);
    }

  next = NULL;
}

Flux::Flux(const Flux& f) :
  format(f.format),  skip(f.skip), scale(f.scale)
{
  fluxName = NULL;
  fileName = NULL;

  if (f.fluxName != NULL)
    {
      fluxName = new char[strlen(f.fluxName)+1];
      memCheck(fluxName,"Flux::Flux() copy constructor: fluxName");
      strcpy(fluxName,f.fluxName);
    }

  if (f.fileName != NULL)
    {
      fileName = new char[strlen(f.fileName)+1];
      memCheck(fileName,"Flux::Flux() copy constructor: fileName");
      strcpy(fileName,f.fileName);
    }
  
  next = NULL;
}

Flux& Flux::operator=(const Flux& f) 
{
  if (this == &f)
    return *this;
  
  scale = f.scale;
  skip = f.skip;
  format = f.format;

  delete fluxName;
  delete fileName;

  fluxName = NULL;
  fileName = NULL;

  if (f.fluxName != NULL)
    {
      fluxName = new char[strlen(f.fluxName)+1];
      memCheck(fluxName,"Flux::operator=(...) : fluxName");
      strcpy(fluxName,f.fluxName);
    }

  if (f.fileName != NULL)
    {
      fileName = new char[strlen(f.fileName)+1];
      memCheck(fileName,"Flux::operator=(...) : fileName");
      strcpy(fileName,f.fileName);
    }
  

  return *this;

}
/****************************
 *********** Input **********
 ***************************/

/***** get flux descriptions ******/
/* called by Input::read(...) */
Flux* Flux::getFlux(istream& input)
{
  char flxName[256], fName[256], type[16];
  double inScale;
  int inSkip, inFormat;

  input >>flxName >> fName >> inScale >> inSkip >> type;
  switch(tolower(type[0]))
    {
    case 'd':
      inFormat = FLUX_D;
      break;
    default:
      error(103,"Invalid flux type: %s", type);
    }
  next = new Flux(inFormat,flxName,fName,inScale,inSkip);
  memCheck(next,"Flux::getFlux(...): next");

  verbose(2,"Added Flux %s from file %s with normalization %g,\
 format code %d, and skipping %d entries.", 
	  flxName,fName,inScale,inFormat,inSkip);

  return next;
}

/****************************
 ********* Preproc **********
 ***************************/

/* cross-reference the fluxes with the intervals
 * read fluxes into interval member objects */
/* called by Input::preproc(...) */
void Flux::xRef(Volume *volList)
{
  Flux *ptr = this;

  VolFlux::setNumFluxes(count());

  verbose(2,"Assigning %d fluxes to each interval",count());

  /* for each flux definition */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      verbose(3,"Assigning flux %s",ptr->fluxName);
      switch (ptr->format)
	{
	case FLUX_T:
	  /* read entire file into intervals */
	  volList->readFlux(ptr->fileName,ptr->skip,ptr->scale);
	  break;
	}
    }

  verbose(3,"Assigned %d fluxes to each interval",count());

}

/****************************
 ********* Utility **********
 ***************************/

/* find a requested flux definition */
int Flux::find(char *srchFlux)
{
  Flux *ptr=this;
  int fluxNum = 0;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      fluxNum++;
      if (!strcmp(ptr->fluxName,srchFlux))
	if (ptr->checkFname())
	  return fluxNum-1;
	else
	  return FLUX_BAD_FNAME;
    }

  return FLUX_NOT_FOUND;
}



/* check the flux file */
int Flux::checkFname()
{
  FILE* textFile = fopen(fileName,"r");

  if (textFile != NULL)
    {
      verbose(5,"Openned flux file %s.",fileName);
      fclose(textFile);
      return TRUE;
    }
  else
    {
      warning(134,"Unable to open flux file %s for flux %s.",fileName,fluxName);
      return FALSE;
    }
 
}

/* count the number of fluxes which are defined */
int Flux::count()
{
  Flux *ptr=this;
  int fluxNum = 0;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      fluxNum++;
    }

  return fluxNum;
}
  


	  
