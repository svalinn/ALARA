/* $Id: Loading.C,v 1.8 1999-08-24 22:06:21 wilson Exp $ */
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

#include "Loading.h"
#include "Mixture.h"
#include "Component.h"
#include "CoolingTime.h"

#include "Calc/topScheduleT.h"

#include "Output/Result.h"

/***************************
 ********* Service *********
 **************************/

Loading::Loading(char *name, char *mxName)
{
  volume = 0;
  zoneName = NULL;
  mixName = NULL;
  mixPtr = NULL;

  if (name != NULL)
    {
      zoneName = new char[strlen(name)+1];
      memCheck(zoneName,"Loading::Loading(...) constructor: zoneName");
      strcpy(zoneName,name);
    }

  if (mxName != NULL)
    {
      mixName = new char[strlen(mxName)+1];
      memCheck(mixName,"Loading::Loading(...) constructor: mixName");
      strcpy(mixName,mxName);
    }

  nComps = 0;
  outputList = NULL;
  total = NULL;

  next = NULL;
  
}

Loading::Loading(const Loading &l)
{
  volume = l.volume;
  mixPtr = l.mixPtr;
  zoneName = NULL;
  mixName = NULL;

  if (l.zoneName != NULL)
    {
      zoneName = new char[strlen(l.zoneName)+1];
      memCheck(zoneName,"Loading::Loading(...) copy constructor: zoneName");
      strcpy(zoneName,l.zoneName);
    }

  if (l.mixName != NULL)
    {
      mixName = new char[strlen(l.mixName)+1];
      memCheck(mixName,"Loading::Loading(...) copy constructor: mixName");
      strcpy(mixName,l.mixName);
    }

  nComps = 0;
  outputList = NULL;
  total = NULL;

  next = NULL;
  
}

Loading::~Loading()
{ 
  delete mixName; 
  delete zoneName; 
  delete [] outputList; 
  delete next; 
}

Loading& Loading::operator=(const Loading &l)
{
  if (this == &l)
    return *this;
  
  volume = l.volume;
  mixPtr = l.mixPtr;
  delete zoneName;
  delete mixName;

  zoneName = NULL;
  mixName = NULL;

  if (l.zoneName != NULL)
    {
      zoneName = new char[strlen(l.zoneName)+1];
      memCheck(zoneName,"Loading::operator=(...): zoneName");
      strcpy(zoneName,l.zoneName);
    }

  if (l.mixName != NULL)
    {
      mixName = new char[strlen(l.mixName)+1];
      memCheck(mixName,"Loading::operator=(...): mixName");
      strcpy(mixName,l.mixName);
    }


  nComps = 0;
  delete [] outputList;
  outputList = NULL;

  delete total;
  total = NULL;

  return *this;

}


/****************************
 *********** Input **********
 ***************************/

/******* get a list of material loadings *******/
/* called by Input::read(...) */
void Loading::getMatLoading(istream& input)
{
  char name[64],token[64];
  Loading *ptr = this;

  verbose(2,"Reading the material loading for this problem.");

  /* read list of zone/mixture x-refs until keyword "end" */
  input >> token;
  while (strcmp(token,"end"))
    {
      /* if ( token[0] != '_') **** WHY WAS THIS ORIGINALLY HERE??????
	{ */
	  input >> name;
	  ptr->next = new Loading(token,name);
	  memCheck(next,"Loading::getMatLoading(...): next");
	  ptr = ptr->next;
          verbose(3,"Adding zone %s with mixture%s.",token,name);
      /* } MATCHES CURIOUS PIECE ABOVE */
      clearComment(input);
      input >> token;
    }
  
  if (ptr->head())
    warning(170,"Material Loading is empty.");

}


/***************************
 ********* xCheck **********
 **************************/

/* cross-check loading: ensure that all referenced mixtures exist */
/* called by Input::xCheck(...) */
void Loading::xCheck(Mixture *mixListHead)
{
  Loading *ptr = this;

  verbose(2,"Checking for all mixtures referenced in material loading.");

  /* for each zone */
  while (ptr->next != NULL)
    {
      ptr=ptr->next;
      if (strcmp(ptr->mixName,"void"))
	{
	  /* search for the mixture referenced in this zone */
	  ptr->mixPtr = mixListHead->find(ptr->mixName);
	  if ( ptr->mixPtr == NULL)
	    error(370, "Zone %s is loaded with a non-existent mixture: %s",
		  ptr->zoneName,ptr->mixName);
	  else
	    {
	      ptr->nComps = ptr->mixPtr->getNComps();
	      ptr->outputList = new Result[ptr->nComps+1];
	    }
	}
      
    }

  verbose(3,"All mixtures referenced in material loading were found.");
}


/*****************************
 ********* PostProc **********
 ****************************/

void Loading::tally(Result *volOutputList, double vol)
{
  int compNum;
  volume += vol;

  /* tally the results for each of the components */
  for (compNum=0;compNum<nComps;compNum++)
    volOutputList[compNum].postProc(outputList[compNum],vol);

  /* tally the total results */
  volOutputList[compNum].postProc(outputList[compNum],vol);
}


void Loading::write(int response, int writeComp, CoolingTime* coolList, 
		    int targetKza)
{
  Loading *head = this;
  Loading *ptr = head;
  int zoneCntr = 0;

  /* for each zone */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      /* write header info */
      cout << "Zone #" << ++zoneCntr << ": " << ptr->zoneName << endl;
      cout << "\tVolume: " << ptr->volume << endl;
      cout << "\tMixture: " << ptr->mixName << endl << endl;

      if (ptr->mixPtr != NULL)
	{
	  /* write the component breakdown if requested */
	  if (writeComp)
	    {
	      /* get the list of components for this mixture */
	      Component *compPtr = ptr->mixPtr->getCompList();
	      int compNum=0;
	      compPtr = compPtr->advance();
	      
	      /* for each component */
	      while (compPtr != NULL)
		{
		  /* write component header */
		  cout << "Component: " << compPtr->getName() << endl;
		  ptr->outputList[compNum].write(response, targetKza,coolList,
						 ptr->total, ptr->volume);

		  compPtr = compPtr->advance();
		  compNum++;
		}
	    }
	  
	  /* if components were written and there is only one */
	  if (writeComp && ptr->nComps == 0)
	    /* write comment refering total to component total */
	    cout << "** Zone totals are the same as those of the single component."
		 << endl << endl;
	  else
	    {
	      /* otherwise write the total response for the zone */
	      cout << "Total" << endl;
	      ptr->outputList[ptr->nComps].write(response, targetKza,coolList,
					    ptr->total, ptr->volume);

	    }
	}
    }
	  
  /** WRITE TOTAL TABLE **/
  /* reset zone pointer and counter */
  ptr = head;
  zoneCntr = 0;

  int resNum,nResults = topScheduleT::getNumCoolingTimes()+1;
  char isoSym[15];

  cout << "Totals for all zones." << endl;

  /* write header for totals */
  coolList->writeTotalHeader("zone");

  /* for each zone */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      cout << ++zoneCntr << "\t";
      if (ptr->mixPtr != NULL)
	for (resNum=0;resNum<nResults;resNum++)
	  {
	    sprintf(isoSym,"%-11.4e ",ptr->total[resNum]);
	    cout << isoSym;
	  }
      cout << "\t" << ptr->zoneName 
	   << " (" << ptr->mixName << ")" << endl;
    }
  coolList->writeSeparator();

  cout << endl << endl;
}



/******************************
 *********** Utility **********
 *****************************/

/* find a given zone in the material loading */
Loading* Loading::findZone(char *srchZone)
{

  Loading *ptr=this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (!strcmp(ptr->zoneName,srchZone))
	return ptr;
    }

  return NULL;
}

/* find a given mixture reference in the material loading */
Loading* Loading::findMix(char *srchMix)
{

  Loading *ptr=this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (!strcmp(ptr->mixName,srchMix))
	return ptr;
    }

  return NULL;
}

/* count number of zones in material loading */
int Loading::numZones()
{
  int numZones = 0;
  Loading *ptr=this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      numZones++;
    }

  return numZones;
}

/* find a named mixture in the list */
void Loading::resetOutList()
{
  int compNum;
  Loading *ptr = this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      ptr->volume = 0;
      if (ptr->outputList != NULL)
	for (compNum=0;compNum<=ptr->nComps;compNum++)
	  ptr->outputList[compNum].clear();
    }
}

