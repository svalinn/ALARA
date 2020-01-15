/* $Id: Loading.C,v 1.28 2005-08-01 21:58:35 wilsonp Exp $ */
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

#include "topScheduleT.h"

#include "Result.h"
#include "Output_def.h"

/***************************
 ********* Service *********
 **************************/

/** When called with no arguments, it creates an blank list head with
    no problem data.  Otherwise, it creates and fills the storage for
    'zoneName' ,'mixName', 'userVol', 'userVolFlag' and initializes
    next to NULL. */
Loading::Loading(const char *name, const char *mxName, bool inUserVolFlag, double inUserVol)
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

  userVolFlag=inUserVolFlag;
  userVol=inUserVol; 
  nComps = 0;
  outputList = NULL;
  total = NULL;

  next = NULL;

}

/** This constructor is identical to default constructor.  Therefore,
    'zoneName' and 'mixName' are copied, but the 'outputList' and
    'total' are initialized to NULL successive list item 'next' is
    not. */
Loading::Loading(const Loading &l)
{
  volume = l.volume;
  mixPtr = l.mixPtr;
  userVol = l.userVol; 
  userVolFlag = l.userVolFlag; 
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

/** This assignment operator behaves similarly to the copy
    constructor.  The correct implementation of this operator must
    ensure that previously allocated space is returned to the free
    store before allocating new space into which to copy the
    object. Note that 'next' is NOT copied, the left hand side object
    will continue to be part of the same list unless explicitly
    changed. */
Loading& Loading::operator=(const Loading &l)
{
  if (this == &l)
    return *this;
  
  volume = l.volume;
  mixPtr = l.mixPtr;
  userVol = l.userVol;
  userVolFlag = l.userVolFlag; 
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
/** The material loadings are expected to appear as a single group
    in the correct order.  This function reads from the first loading
    until keyword 'end'. */
void Loading::getMatLoading(istream& input)
{
  char name[64],token[64],usv[64];
  bool busv; 
  Loading *ptr = this;
  bool isusv(char* Usv);	

  verbose(2,"Reading the material loading for this problem.");

  /* read list of zone/mixture x-refs until keyword "end" */
  input >> token;
  while (strcmp(token,"end"))
    {
      input >> name;  
      clearComment(input);
      input >> usv;			 
      clearComment(input);
      busv=isusv(usv)?TRUE:FALSE;
      ptr->next = new Loading(token,name,busv,strtod(usv,NULL));
      memCheck(next,"Loading::getMatLoading(...): next");
      ptr = ptr->next;
      verbose(3,"Adding zone %s with mixture%s.",token,name);
      if (busv)
	   input >> token;
      else
           strcpy(token,usv);
    }
  
  if (ptr->head())
    warning(170,"Material Loading is empty.");

}

//********* Function isusv declared in Loading::getMatLoading**********
bool isusv(char* Usv)	
{
	//Checks wheter argument points to real number. If this were the case
	//then this is assume to be userVol and the function returns TRUE.
  
   	char* strptr=new char[64];
	char* *pcharptr=&strptr;

	strtod(Usv,pcharptr);
	if (strptr[0]=='\0')
		return TRUE;
	else
		return FALSE;
}
/******* get a list of material loadings *******/
/* called by Input::read(...) */
/** These lists are cross-referenced with the  master list in
    xCheck(). */
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

/* called by Input::xCheck(...) */
/** If a mixture does not exist, it will generate an error and the
    program will halt.  The argument is a pointer to a Mixture object,
    and should be the head of the mixture list. */
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

/** The tallying is weighted by the second argument.  This is used to
    tally the results of each interval in to the total zone results,
    weighted by the interval volume. */
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


/** The first argument indicates which kind of response is being
    written and the second indicates whether a mixture component
    breakdown was requested.  The third argument points to the list of
    after-shutdown cooling times.  The fourth argument indicates the
    kza of the target isotope for a reverse calculation and is simply
    passed on the the Result::write().  The final argument indicates
    what type of normalization is being used, so that the correct
    output information can be given. */
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
      debug(5,"Loading::userVol=%f",ptr->userVol);
      if (ptr->mixPtr != NULL)
	{
	  if (normType > 0)
	    cout << "\tRelative Volume: " << ptr->volume << endl;
	  else
	    cout << "\tMass: " << ptr->volume*ptr->mixPtr->getTotalDensity()
		 << endl;

	  cout << "\tContaining mixture: " << ptr->mixName << endl << endl;

	  /* write the component breakdown if requested */
	  if (writeComp && response != OUTFMT_SRC)
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
		  
		  if (normType < 0)
		    {
		      cout
		           << "\tVolume Fraction: " << volFrac
		           << "\tRelative Volume: " << volume_mass;
		      density = compPtr->getDensity();
		      volume_mass *= density;
		      cout
			<< "\tDensity: " << density 
			<< "\tMass: " << volume_mass;
		    }
		  else if (normType == OUTNORM_VOL_INT)
		    {
		      /* The loading responses are volume weighted sums already.
			 For volume integrated results, don't renormalize */
		      cout
		           << "\tVolume Fraction: " << volFrac
		           << "\tAbsolute Volume: " << ptr->userVol;
		      volume_mass /= ptr->userVol;
		      cout << "\tVolume Integrated ";
		    }
		  else
		    {
		      cout
		           << "\tVolume Fraction: " << volFrac
		           << "\tRelative Volume: " << volume_mass;
		    }

		  
		  cout << endl;

		  ptr->outputList[compNum].write(response,targetKza,ptr->mixPtr,
						 coolList,ptr->total,volume_mass);

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
	      volume_mass = ptr->volume * volFrac;

	      cout << "Total (All constituents) " << endl;

	      cout << "\tCOMPACTED" << endl;

	      
	      if (normType < 0)
		{
		  density = ptr->mixPtr->getTotalDensity();
		  /* different from constituent: mixture densities 
		     already take volume fraction into account */
	          cout 
		    << "\tVolume Fraction: " << volFrac
		    << "\tRelative Volume: " << volume_mass;
		  volume_mass = ptr->volume * density;
		  cout
		    << "\tDensity: " << density 
		    << "\tMass: " << volume_mass;
		}
	      else if (normType == OUTNORM_VOL_INT)
		{
		  /* The loading responses are volume weighted sums already.
		     For volume integrated results, don't renormalize */
	          cout 
		    << "\tVolume Fraction: " << volFrac
		    << "\tAbsolute Volume: " << ptr->userVol;
		  volume_mass /= ptr->userVol;
		  cout << "\tVolume Integrated ";
		}
	      else
		{
	          cout 
		    << "\tVolume Fraction: " << volFrac
		    << "\tRelative Volume: " << volume_mass;
		}
	      
	      cout << endl;

	      ptr->outputList[ptr->nComps].write(response, targetKza, ptr->mixPtr,
						 coolList, ptr->total, volume_mass);

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

  cout << Result::getReminderStr() << endl;

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

/** If found, returns a pointer to that zone loading description,
    otherwise, NULL. */
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

/** If found, returns a pointer to that zone loading description,
    otherwise, NULL.  Note: this returns the first occurence after a the
    object through which it is called - if called through the list head,
    this is the absolute occurence. By successive calls through the object
    returned by the previous call, this will find all the occurences. */
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


