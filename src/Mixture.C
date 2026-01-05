/* $Id: Mixture.C,v 1.37 2008-08-06 17:38:09 phruksar Exp $ */
/* (potential) File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */

#include "Mixture.h"
#include "Component.h"
#include "Loading.h"
#include "Volume.h"
#include "CoolingTime.h"

#include "Root.h"

#include "topScheduleT.h"

#include "Result.h"
#include "Output_def.h"
#include "GammaSrc.h" 
#include "VolFlux.h"

/***************************
 ********* Service *********
 **************************/

/** When called without arguments, the default constructor creates a
    blank list head with no problem data.  Otherwise, it creates and
    fills the storage for 'mixName', initializes a new component list
    and volume list, and sets the 'rootList' and 'next' pointer to
    NULL. */
Mixture::Mixture(const char *name)
{
  volume = 0;
  userVol = 0.0;
  mixName = NULL;
  if (name != NULL)
    {
      mixName = new char[strlen(name)+1];
      memCheck(mixName,"Mixture::Mixture(...) constructor: mixName");
      strcpy(mixName,name);
    }

  compListHead = new Component(COMP_HEAD);
  targetCompListHead = new Component(COMP_HEAD);
  nComps = 0;
  totalDensity = 0;
  totalNDensity = 0;
  volFraction = 0;

  volList = new Volume(VOL_HEAD);

  rootList = NULL;
  targetList = NULL;
  outputList = NULL;
  total = NULL;

  next = NULL;
}

/** This constructor copies 'volume' and 'mixName', and initializes
    'compListHead', 'targetCompListHead', and 'volList', setting other
    variables to NULL. */
Mixture::Mixture(const Mixture &m)
{
  volume = m.volume;
  userVol = m.userVol; 
  mixName = NULL;
  if (m.mixName != NULL)
    {
      mixName = new char[strlen(m.mixName)+1];
      memCheck(mixName,"Mixture::Mixture(...) copy constructor: mixName");
      strcpy(mixName,m.mixName);
    }

  compListHead = new Component(COMP_HEAD);
  targetCompListHead = new Component(COMP_HEAD);
  nComps = 0;
  totalDensity = 0;
  totalNDensity = 0;
  volFraction = 0;

  volList = new Volume(VOL_HEAD);

  rootList = NULL;
  targetList = NULL;
  outputList = NULL;
  total = NULL;

  next = NULL;
}

/** Destructor deletes storage for 'mixName', component list
    and rootList, but not the interval list.  It then destroys the
    rest of the mixture list by deleting 'next'. */
Mixture::~Mixture()
{ 
  delete[] mixName; 
  delete compListHead; 
  delete targetCompListHead;
  delete rootList;
  delete targetList;
  delete volList;
  delete [] outputList;
  delete next;
  next = NULL;
}

/** This assignment operator behaves similarly to the copy
    constructor.  The correct implementation of this operator must
    ensure that previously allocated space is returned to the free
    store before allocating new space into which to copy the
    object. Note that 'next' is NOT copied, the left-hand-side object
    will continue to be part of the same list unless explicitly
    changed. */
Mixture& Mixture::operator=(const Mixture &m)
{

  if (this == &m)
    return *this;

  volume = m.volume;
  userVol = m.userVol; 
  totalDensity = m.totalDensity;
  totalNDensity = m.totalNDensity;
  volFraction = m.volFraction;
  delete mixName;
  mixName = NULL;
  if (m.mixName != NULL)
    {
      mixName = new char[strlen(m.mixName)+1];
      memCheck(mixName,"Mixture::operator=(...): mixName");
      strcpy(mixName,m.mixName);
    }

  delete compListHead;
  compListHead = new Component(COMP_HEAD);
  delete targetCompListHead;
  targetCompListHead = new Component(COMP_HEAD);

  delete volList;
  volList = new Volume(VOL_HEAD);

  delete rootList;
  rootList = NULL;
  delete targetList;
  targetList = NULL;

  delete [] outputList;
  nComps = 0;
  outputList = NULL;
  
  delete total;
  total = NULL;

  return *this;


}

/***************************
 *********** Input *********
 ***************************/

/** It reads each Component in the list up to keyword 'end', and returns
    a pointer to the newly created Mixture object. */
Mixture* Mixture::getMixture(istream &input)
{
  char name[256], token[64];
  int type;

  input >> name;
  next = new Mixture(name);
  memCheck(next,"Mixture::getMixture(...): next");

  Mixture *mixPtr = next;

  /* read a list of componenets until keyword "end" */
  Component* compList = mixPtr->compListHead;
  Component* targetCompList = mixPtr->targetCompListHead;
  mixPtr->nComps = 0;

  verbose(2,"Reading constituent list for Mixture %s with constituents:",name);
  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      mixPtr->nComps++;

      switch(tolower(token[0]))
	{
	case 'm':
	  debug(2,"Creating new Constituent object of material type");
	  type = COMP_MAT;
	  break;
	case 'e':
	  debug(2,"Creating new Constituent object of element type");
	  type = COMP_ELE;
	  break;
/*	case 'i':
	  debug(2,"Creating new Constituent object of isotope type");
	  type = COMP_ISO;
	  break; */
	case 'l':
	  debug(2,"Creating new Constituent object of similar type");
	  type = COMP_SIM;
	  break;
	case 't':
	  Chain::modeReverse();
	  /* don't count this in component list */
	  mixPtr->nComps--;
	  clearComment(input);
	  input >> token;
	  switch(tolower(token[0]))
	    {
	    case 'e':
	      type = TARGET_ELE;
	      break;
	    case 'i':
	      type = TARGET_ISO;
	      break;
	    default:
	      error(180,"Target materials for reverse calculations can only be elements or isotopes and not '%s'",token);
	    }
	  break;
	default:
	  error(181,"Invalid material constituent: %s", token);
	}
      if (type <= COMP_SIM)
	/* add each component to the list */
	compList = compList->getComponent(type,input,mixPtr);
      else
	targetCompList = targetCompList->getComponent(type,input,mixPtr);

      clearComment(input);
      input >> token;
    }
  
  if (compList->head())
    warning(182,"Mixture %s has no constituents",name);
  
  /* treat a single component as if there is no component:
   * total only */
  if (mixPtr->nComps == 1)
    mixPtr->nComps = 0;
  mixPtr->outputList = new Result[mixPtr->nComps+1];
  
  debug(1,"Finished reading Mixture %s.", name);
  return mixPtr;         
}

/***************************
 ********* xCheck **********
 **************************/

/** It does this by ensuring that each named Mixture referenced in a
    component of type COMP_SIM exists in the list of mixtures.  If an
    inconsitency is found, an error results.  This should be called through
    the head of the mixture list as it internally loops through each of the
    Mixture list elements. */
void Mixture::xCheck()
{
  Mixture *head = this;
  Mixture *ptr = this;
  Component *current;

  verbose(2,"Checking for all internally referenced mixtures.");

  /* for each mixture */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      /* search the component list for type 'l' = similar */
      current = ptr->compListHead->exists(COMP_SIM);
      /* for each type 'l' */
      while(current != NULL)
	{
	  /* search for the referenced name */
	  if (head->find(current->getName()) == NULL)
	      error(380, "Constituent type 'l' of mixture %s references a non-existent mixture: %s",
		      ptr->mixName,current->getName());

	  current = current->exists(COMP_SIM);
	}
    }

  verbose(3,"All internally referenced mixtures were found.");
}


/****************************
 ********* Preproc **********
 ***************************/

/** Once copySim() is complete, if a Mixture is not referenced in a
    material Loading, a warning is generated and the Mixture is
    deleted.  This loops internally through all the Mixture elements,
    and should be called through the head to the Mixture list with
    the head of the Loading list as its argument. */
void Mixture::removeUnused(Loading *loadList)
{
  Mixture *head = this;
  Mixture *prev, *ptr = this;

  verbose(2,"Replacing all 'similar' constituents and removing unused mixtures.");

  while (ptr->next != NULL)
    {
      prev = ptr;
      ptr = ptr->next;
      /* expand "similar" entries with current mixture definition */
      head->copySim(ptr);

      /* if this mixture is not used in the material loading, remove it */
      if (loadList->findMix(ptr->mixName) == NULL)
	{
	  /* remove current mixture def from list */
	  prev->next = ptr->next;
	  warning(580,"Removing mixture %s not used in any zones.",
		  ptr->mixName);

	  /* this ensures that the entire list is not deleted */
	  ptr->next = NULL;
	  delete ptr;

	  /* go back to last item for testing while loop */
	  ptr = prev;
	}
    }

  verbose(3,"Mixture list has been cleaned up and 'similar' constituents expanded.");
}

/** This is called by removeUnused() (above) once for each Mixture. */
void Mixture::copySim(Mixture *cpyPtr)
{
  Mixture *ptr = this;
  Component *current;
  
  verbose(3,"Replacing all constituents similar to %s with their expanded definition.", cpyPtr->mixName);

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      
      current = ptr->compListHead->exists(COMP_SIM);
      while (current != NULL)
	{
	  /* if this component is similar to mixture of interest */
	  if (!strcmp(cpyPtr->mixName,current->getName()))
	    {
	      /* replace the similar component */
	      current = current->replaceSim(cpyPtr->compListHead);
	      verbose(4,"Replaced constituent of mixture %s with definition of mixture %s",
		      ptr->mixName,cpyPtr->mixName);
	    }
	  current = current->exists(COMP_SIM);
	}
    }

  verbose(4,"All constituents similar to %s have been replaced.", cpyPtr->mixName);
}

/** It does this by adding the Volume referenced in the only argument to
    the list of intervals. This is done by simply passing this reference
    back to a function of the Volume list head object, if the list has begun\
,
otherwise, setting the Volume list head to point to this same object. */
void Mixture::xRef(Volume *volPtr)
{
  volList->addMixList(volPtr);
}

/** This function should be called through the head of the Mixture
    list. As each rootList is created, it is merged with the
    top-level/master root list, massed by reference to point as the only
    argument. */
void Mixture::makeRootList(Root *&masterRootList)
{
  Mixture *ptr = this;

  verbose(2,"Making list of root isotopes.");
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      /* expand the components into a root list */
      verbose(3,"Expanding mixture %s",ptr->mixName);
      ptr->rootList = ptr->compListHead->expand(ptr);

      // Calculate and store pre-irradiation number densities
      Root *root = ptr->rootList->getNext();
      while (root != NULL)
      {
        double nd_cm3 = root->mixConc(ptr); // atoms/cm^3
        double density = ptr->totalDensity; // g/cm^3

        // Calculate number density in atoms/kg
        double nd_per_kg = 0.0;
        if (density > 0.0)
          nd_per_kg = nd_cm3 * 1e3 / density; // atoms/kg

        // Store the pre-irradiation ND in this root
        root->setPreIrradND(ptr, nd_per_kg);
        root = root->getNext();
      }

      switch(NuclearData::getMode())
	{
	case MODE_FORWARD:
	  /* merge this root list into the master */
	  masterRootList = masterRootList->merge(ptr->rootList);
	  verbose(4,"Merged rootlist for mixture %s to master root list.",
		  ptr->mixName);
	  break;
	case MODE_REVERSE:
	  /* expand the target components into a target list */
	  ptr->targetList = ptr->targetCompListHead->expand(ptr);
	  /* merge this root list into the master */
	  masterRootList = masterRootList->merge(ptr->targetList);
	  verbose(4,"Merged targetList for mixture %s to master root list.",
		  ptr->mixName);
	  break;
	}
    }

  verbose(3,"Expanded all mixtures to master root list.");

}

/****************************
 ********* Solution *********
 ***************************/

/** The reference flux contained in this volume is checked and updated
    against each interval. */
void Mixture::refFlux(Volume *refVolume)
{
  volList->refFlux(refVolume);
}

/** The chain is solved on the schedule for each interval. */
void Mixture::solve(Chain* chain, topSchedule* schedule)
{
  volList->solve(chain,schedule);
}

/** This starts the process of writing the dump file using the list of
    intervals which contain this mixture. */
void Mixture::writeDump()
{
  volList->writeDump();
}

/*****************************
 ********* PostProc **********
 ****************************/

void Mixture::readDump(int kza)
{
  volList->readDump(kza);
}

/** The tallying is weighted by the second argument.  This is used to
    tally the results of each interval in to the total mixture results,
    weighted by the interval volume. */
void Mixture::tally(Result *volOutputList, double vol)
{
  int compNum;
  volume += vol;

  for (compNum=0;compNum<nComps;compNum++)
    volOutputList[compNum].postProc(outputList[compNum],vol);

  volOutputList[compNum].postProc(outputList[compNum],vol);
}

/** The first argument indicates which kind of response is
    being written, the second indicates whether a mixture component
    breakdown was requested, and the third points to the list of
    after-shutdown cooling times. The fourth argument indicates the
    kza of the target isotope for a reverse calculation and is simply
    passed on the the Result::write().  The final argument indicates
    what type of normalization is being used, so that the correct
    output information can be given. */
void Mixture::write(int response, int writeComp, CoolingTime* coolList,
		    int targetKza, int normType)
{
  Mixture *head = this;
  Mixture *ptr = head;
  int mixCntr = 0;
  double volFrac, volume_mass, density;

  /* for each mixture */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      /* write header information */
      cout << endl;
      cout << "Mixture #" << ++mixCntr << ": " << ptr->mixName << endl;
      debug(5,"Mixture::userVol=%f",ptr->userVol);
      if (normType > 0)
	cout << "\tRelative Volume: " << ptr->volume << endl;
      else
	cout << "\tMass: " << ptr->volume*ptr->totalDensity << endl;

      /* write the component breakdown if requested */
      if (writeComp && response != OUTFMT_SRC)
	{
	  /* get the list of components for this mixture */
	  Component *compPtr = ptr->compListHead;
	  int compNum=0;
	  compPtr = compPtr->advance();

	  /* for each component */
	  while (compPtr != NULL)
	    {
	      volFrac = compPtr->getVolFrac();
	      volume_mass = ptr->volume * volFrac;

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
		  /* The mixture responses are volume weighted sums already.
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

	      ptr->outputList[compNum].write(response,targetKza,this,
					     coolList,ptr->total,volume_mass);

	      compPtr = compPtr->advance();
	      compNum++;
	    }
	}
      
      volFrac = ptr->volFraction;

      /* if components were written and there is only one */
      if (writeComp && ptr->nComps == 0 && volFrac == 1.0)
	/* write comment refering total to component total */
	cout << "** Interval totals are the same as those of the single constituent."
	     << endl << endl;
      else
	{
	  /* otherwise write the total response for the zone */
	  volume_mass = volFrac*ptr->volume;
	  
	  /* write component header */
	  cout << "Total (All constituents) " << endl;

	  cout << "\tCOMPACTED" << endl;

	  
	  if (normType < 0)
	    {
	      /* different from constituent: mixture densities 
		 already take volume fraction into account */
	      cout 
	        << "\tVolume Fraction: " << volFrac
	        << "\tRelative Volume: " << volume_mass;
	      volume_mass = ptr->totalDensity * ptr->volume;
	      cout
		<< "\tDensity: " << ptr->totalDensity
		<< "\tMass: " << volume_mass;
	    }
	  else if (normType == OUTNORM_VOL_INT)
	    {
	      /* The mixture responses are volume weighted sums already.
		 For volume integrated results, don't renormalize */
	      cout 
	        << "\tVolume Fraction: " << volFrac
	        << "\tAbsolute Volume: " << ptr->userVol;
	      volume_mass /=ptr->userVol;
	      cout << "\tVolume Integrated ";
	    }
	  else
	    {
	      cout 
		<< "\tVolume Fraction: " << volFrac
		<< "\tRelative Volume: " << volume_mass;
	    }
	  
	  cout << endl;
	      
	  ptr->outputList[ptr->nComps].write(response,targetKza,this,
					     coolList,ptr->total,volume_mass);

	}
    }
	  

  /** WRITE TOTAL TABLE **/
  /* reset mixture pointer and counter */
  ptr = head;
  mixCntr = 0;

  int resNum,nResults = topScheduleT::getNumCoolingTimes()+1;
  char isoSym[15];

  cout << endl;
  cout << "Totals for all mixtures." << endl;

  cout << Result::getReminderStr() << endl;

  /* write header for totals */
  coolList->writeTotalHeader("mixture");

  /* for each mixture */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      cout << ++mixCntr << "\t";

      for (resNum=0;resNum<nResults;resNum++)
	{
	  sprintf(isoSym,"%-11.4e ",ptr->total[resNum]);
	  cout << isoSym;
	}
      cout << "\t" << ptr->mixName << endl;
    }
  coolList->writeSeparator();

  cout << endl << endl;
}

double Mixture::getDoseConv(int kza, GammaSrc* contactDose)
{
  if (!doseConvCache.count(kza))
    doseConvCache[kza] = contactDose->calcDoseConv(kza,gammaAttenCoef);
  
  return doseConvCache[kza];

}


void Mixture::setGammaAttenCoef(int nGroups, ifstream& gAttenData)
{

  const int MaxEle = 106;
  Mixture *ptr = this;
  Root *root = NULL;
  double *gammaAttenData = NULL;
  double density, totalDens, interp;
  
  int gammaAttenZ[MaxEle], kza, Z, A;
  int gNum, idx, numEle;


  gammaAttenData = new double[nGroups*MaxEle];

  numEle = 0;
  /* read all attenuation coeffcient data */
  clearComment(gAttenData);
  gAttenData >> gammaAttenZ[numEle];
  while (!gAttenData.eof())
    {
      clearComment(gAttenData);
      for (gNum=0;gNum<nGroups;gNum++)
	gAttenData >> gammaAttenData[numEle*nGroups + gNum];

      numEle++;

      clearComment(gAttenData);
      gAttenData >> gammaAttenZ[numEle];
      
    }

  gammaAttenZ[numEle] = 999999;
      
  /* for each mixture in problem */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      /* create & initialize array */
      ptr->gammaAttenCoef = new double[nGroups];
      for (gNum=0;gNum<nGroups;ptr->gammaAttenCoef[gNum++]=0) ;

      totalDens = 0;

      /* for each root in mixture */
      root = ptr->rootList->getNext();
      while (root != NULL)
	{
	  kza = root->getKza();
	  Z = kza/10000;
	  A = (kza - 10000*Z)/10;

	  /* get total density of this root */
	  density = root->mixConc(ptr)*A;
	  totalDens += density;

	  /* find data for this Z */
	  idx=-1;
	  while (gammaAttenZ[++idx] < Z) ;
	  if (gammaAttenZ[idx] == Z)
	    for (gNum=0;gNum<nGroups;gNum++)
	      ptr->gammaAttenCoef[gNum] += density*gammaAttenData[idx*nGroups+gNum];
	  else
	    /* if no data, interpolate */
	    if (idx > 0)
	      {
		/* if we are above last data point, extrapolate */
		if (idx == numEle)
		  idx--;
		interp = float(Z - gammaAttenZ[idx-1])/
		  float(gammaAttenZ[idx]-gammaAttenZ[idx-1]);
		for (gNum=0;gNum<nGroups;gNum++)
		  ptr->gammaAttenCoef[gNum] += density*
		    ( gammaAttenData[  idx  *nGroups+gNum] * interp      
		     +gammaAttenData[(idx-1)*nGroups+gNum] * (1 - interp) );
	      }

	  root = root->getNext();

	}

      for (gNum=0;gNum<nGroups;gNum++)
	ptr->gammaAttenCoef[gNum] = ptr->gammaAttenCoef[gNum]/totalDens;

    }
      
  delete gammaAttenData;
  
}


/****************************
 ********* Utility **********
 ***************************/

/** It does this with 'kza' by matching the first argument, and then by
    searching that object's MixCompRef references for the next component of
    this mixture that contains this root. The last match is indicated by the
    third argument (and the 'this' pointer).  The reference argument
    (double&) is updated with the density of the root isotope in the
    appropriate component, when the match is found. */
Component* Mixture::getComp(int kza,double &density, Component *lastComp)
{
  Root *root = rootList->find(kza);

  if (root)
    return root->getComp(density,this,lastComp);
  else
    {
      density = 0;
      return NULL;
    }
}

int Mixture::getCompNum(Component* compPtr)
{
  return compListHead->getCompNum(compPtr);

}

/** The name is passed as the only argument.  If found, a pointer to the
    appropriate Mixture object is returned, otherwise, NULL. */
Mixture* Mixture::find(char* srchName)
{

  Mixture *ptr = this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (!strcmp(ptr->mixName,srchName))
	return ptr;
    }

  return NULL;
}

void Mixture::resetOutList()
{
  int compNum;
  Mixture *ptr = this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      ptr->volume = 0;
      for (compNum=0;compNum<=ptr->nComps;compNum++)
	ptr->outputList[compNum].clear();
    }
}

