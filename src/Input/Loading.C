/* $Id: Loading.C,v 1.18 2000-02-17 16:21:38 wilson Exp $ */
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
#include "Output/Output_def.h"

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
      input >> name;
      ptr->next = new Loading(token,name);
      memCheck(next,"Loading::getMatLoading(...): next");
      ptr = ptr->next;
      verbose(3,"Adding zone %s with mixture%s.",token,name);
      clearComment(input);
      input >> token;
    }
  
  if (ptr->head())
    warning(170,"Material Loading is empty.");

}

/******* get a list of material loadings *******/
/* called by Input::read(...) */
void Loading::getSolveList(istream& input)
{
  char token[64];
  Loading *ptr = this;

  verbose(2,"Reading the zones to solve/skip for this problem.");

  /* read list of zone/mixture x-refs until keyword "end" */
  input >> token;
  while (strcmp(token,"end"))
    {
      ptr->next = new Loading(token,"on");
      memCheck(next,"Loading::getSolveList(...): next");
      ptr = ptr->next;
      verbose(3,"Adding zone %s to solve/skip list.",token);
      clearComment(input);
      input >> token;
    }
  
}


/***************************
 ********* xCheck **********
 **************************/

/* cross-check loading: ensure that all referenced mixtures exist */
/* called by Input::xCheck(...) */
void Loading::xCheck(Mixture *mixListHead, Loading *solveList, Loading *skipList)
{
  Loading *ptr = this;

  verbose(2,"Checking for all mixtures referenced in material loading.");

  /* for each zone */
  while (ptr->next != NULL)
    {
      ptr=ptr->next;

      /* check for this zone in explicit solve list or skip list */
      if ( ( solveList->nonEmpty() && solveList->findZone(ptr->zoneName)==NULL ) 
	   || skipList->findZone(ptr->zoneName)!=NULL )
	{
	  delete ptr->mixName;
	  ptr->mixName = new char[5];
	  strcpy(ptr->mixName,"void");
	}
      else if (strcmp(ptr->mixName,"void"))
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
		    int targetKza, int normType)
{
  Loading *head = this;
  Loading *ptr = head;
  int zoneCntr = 0;
  double volFrac, volume_mass, density;

  /* for each zone */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      /* write header info */
      cout << endl;
      cout << "Zone #" << ++zoneCntr << ": " << ptr->zoneName << endl;

      if (ptr->mixPtr != NULL)
	{
	  if (normType > 0)
	    cout << "\tVolume: " << ptr->volume << endl;
	  else
	    cout << "\tMass: " << ptr->volume*ptr->mixPtr->getTotalDensity()
		 << endl;

	  cout << "\tContaining mixture: " << ptr->mixName << endl << endl;

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

		  volFrac = compPtr->getVolFrac();
		  volume_mass = ptr->volume*volFrac;

		  /* write component header */
		  cout << "Constituent: " << compPtr->getName() << endl;
		  cout
		       << "\tVolume Fraction: " << volFrac
		       << "\tVolume: " << volume_mass;
		  
		  if (normType < 0)
		    {
		      density = compPtr->getDensity();
		      volume_mass *= density;
		      cout
			<< "\tDensity: " << density 
			<< "\tMass: " << volume_mass;
		    }
		  
		  cout << endl;

		  ptr->outputList[compNum].write(response,targetKza,coolList,
						 ptr->total,volume_mass);

		  compPtr = compPtr->advance();
		  compNum++;
		}
	    }

	  volFrac = ptr->mixPtr->getVolFrac();
	  /* if components were written and there is only one */
	  if (writeComp && ptr->nComps == 0 && volFrac == 1.0)
	    /* write comment refering total to component total */
	    cout << "** Zone totals are the same as those of the single constituent."
		 << endl << endl;
	  else
	    {
	      /* otherwise write the total response for the zone */
	      if (response != OUTFMT_WDR)
		volFrac = 1.0;

	      volume_mass = ptr->volume * volFrac;

	      cout << "Total (All constituents) " << endl;
	      if (response != OUTFMT_WDR)
		cout << "\tNON-Compacted" << endl;
	      else
		cout << "\tCOMPACTED" << endl;

	      cout 
		<< "\tVolume Fraction: " << volFrac
		<< "\tVolume: " << volume_mass;
	      
	      if (normType < 0)
		{
		  density = ptr->mixPtr->getTotalDensity();
		  /* different from constituent: mixture densities 
		     already take volume fraction into account */
		  volume_mass = ptr->volume * density;
		  cout
		    << "\tDensity: " << density 
		    << "\tMass: " << volume_mass;
		}

	      cout << endl;

	      ptr->outputList[ptr->nComps].write(response, targetKza,coolList,
					    ptr->total, volume_mass);

	    }
	}
    }
	  
  /** WRITE TOTAL TABLE **/
  /* reset zone pointer and counter */
  ptr = head;
  zoneCntr = 0;

  int resNum,nResults = topScheduleT::getNumCoolingTimes()+1;
  char isoSym[15];

  cout << endl;
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

/* reset the output info for next target */
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


