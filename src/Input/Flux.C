/* $Id: Flux.C,v 1.11 2002-09-25 07:22:00 wilsonp Exp $ */
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

extern "C" float rt2al_(double *freg, int *nInts, int *nGrps, int *skip,
			char *fname,int *fnamelen, int *err);

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
    
    case 'r':
      // binary format (read from binary file)
      inFormat = FLUX_R;
      break;
    
    case 'd':
      // default format (read from text file)
      inFormat = FLUX_D;
      break;
    default:
      error(140,"Invalid flux type: %s", type);
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
  int numVols = volList->count();
  int numGrps = VolFlux::getNumGroups();
  double temp;

  VolFlux::setNumFluxes(count());

  verbose(2,"Assigning %d fluxes to each interval",count());

  // Dynamically Create Matrix
  double **FluxMatrix = new double*[numVols];
  double *MatrixStorage = new double[numVols*numGrps];

  for(int i = 0; i < numVols; i++)
    FluxMatrix[i] = &MatrixStorage[i*numGrps];

  /* for each flux definition */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      verbose(3,"Assigning flux %s",ptr->fluxName);

      switch (ptr->format)
	{
	case FLUX_D:
	  {
	    /* Default: Reads data from fluxin file */

	    // Open Input File
	    ifstream FluxData(ptr->fileName);

	    // Skip appropriate number of Volumes
	    if(ptr->skip > 0)
	      for(int i = 0; i < ptr->skip; i++)
		for(int j = 0; j < numGrps; j++)
		  FluxData >> temp;

	    if(FluxData.eof())
	      error(622,"Flux file %s does not contain enough data.",
		    ptr->fileName);

	    // Load data from FluxData
	    for(int x = 0; x < numVols; x++)
	    {
	      for(int y = 0; y < numGrps; y++)
	      {
		if(FluxData.eof())
		  error(622,"Flux file %s does not contain enough data.",
			ptr->fileName);

		FluxData >> FluxMatrix[x][y];
	      }
	    }
	    FluxData.close();
	    break;
	  }

	case FLUX_R:
	  {
	    /* Binary: reads data from binary file */
	    char *fname = new char[strlen(ptr->fileName)];
	    strcpy(fname,ptr->fileName);
	    
	    int length = strlen(fname) + 1, err;

	    rt2al_(MatrixStorage, &numVols, &numGrps, &ptr->skip,
		   fname,&length,&err);
	    
	    switch(err)
	      {
	      case 0:
		// Normal
		break;

	      case 1:
		// Data not 1 dimensional
		break;
		
	      case 2:
		// Not enough data in binary file
		break;
	      }
		
	    break;
	  }
	};
      
      volList->storeMatrix(FluxMatrix,ptr->scale);
    }

  delete FluxMatrix;

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
      warning(340,"Unable to open flux file %s for flux %s.",fileName,fluxName);
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
  


	  
