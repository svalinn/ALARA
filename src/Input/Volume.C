/* $Id: Volume.C,v 1.35 2003-01-14 22:09:23 wilsonp Exp $ */
#include "Volume.h"
#include "Loading.h"
#include "Geometry.h"
#include "Mixture.h"
#include "Component.h"
#include "CoolingTime.h"
#include "Output/GammaSrc.h"
#include "Norm.h"

#include "Chains/Chain.h"
#include "Chains/Root.h"

#include "Calc/VolFlux.h"
#include "Calc/topSchedule.h"
#include "Calc/topScheduleT.h"

#include "Output/Result.h"
#include "Output/Output_def.h"

// Declare Static Members
TempLibType Volume::specLib;
TempLibType Volume::rangeLib;
int* Volume::energyRel = NULL;

/***************************
 ********* Service *********
 **************************/

void Volume::init()
{
  volume = 1;
  userVol=0;
  norm = 1;
 
  zoneName = NULL;
  zonePtr = NULL;
  mixPtr = NULL;
  next = NULL;
  mixNext = NULL;
  adjConv = NULL;

  fluxHead = new VolFlux;
  flux = fluxHead;

  schedT = NULL;

  nComps = 0;
  outputList = NULL;

  total = NULL;
}

void Volume::deinit()
{
  delete zoneName; 
  delete fluxHead; 
  delete schedT; 
  delete adjConv;
  delete [] outputList;
}  

/** This constructor always call init.  When called without arguments,
    the default constructor creates a blank list with no problem data.
    Otherwise, it sets the volume of the interval, and creates and
    fills the storage for 'zoneName'. */
Volume::Volume(double vol, char *name)
{
  init();

  volume=vol;

  if (name != NULL)
    {
      zoneName = new char[strlen(name)+1];
      memCheck(zoneName,"Volume::Volume(...) constructor: zoneName");
      strcpy(zoneName,name);
      
    }

}

/** This is identical to default constructor, the first argument used
    as the volume and accessing the name of the zone from the second
    argument, a pointer to the zone where this interval is located.
    The 'zonePtr' is then initialized with this second argument. */
Volume::Volume(double vol, Loading *loadPtr)
{
  init();
  volume=vol;
  zonePtr = loadPtr;

  if (zonePtr->getName() != NULL)
    {
      zoneName = new char[strlen(zonePtr->getName())+1];
      memCheck(zoneName,"Volume::Volume(...) alternate constructor: zoneName");
      strcpy(zoneName,zonePtr->getName());
      
    }


}

/** This constructor starts by calling init(), and then copies the
    'volume', 'norm', 'zonePtr', 'mixPtr', and 'zoneName' from the
    passed argument. */
Volume::Volume(const Volume& v)
{
  init();

  volume = v.volume;
  norm = v.norm;
  zonePtr = v.zonePtr;
  mixPtr = v.mixPtr;
  userVol=v.userVol;
  // Need to confirm desired behavior of copy constructor for this variable
  adjConv=v.adjConv;

  if (v.zoneName != NULL)
    {
      zoneName = new char[strlen(v.zoneName)+1];
      memCheck(zoneName,"Volume::Volume(...) copy constructor: zoneName");
      strcpy(zoneName,v.zoneName);
      
    }
}

Volume::Volume(Root *rootPtr,topSchedule* top)
{
  init();

  volume = 0;
  
  rootPtr->refFlux(this);
  
  switch(VolFlux::getRefFluxType()) {
  case REFFLUX_VOL_AVG:
    fluxHead->scale(1.0/volume);
    break;
  case REFFLUX_MAX:
  default:
    volume = 1.0;
    break;
  }
  
  schedT = new topScheduleT(top);

}

Volume::~Volume() 
{
  deinit();
  delete next; 
}

/** This assignment operator first calls deinit() and init() to
    reinitialize its storage.  It then behaves similarly to the copy
    constructor.  Note that 'next' and 'mixNext' are NOT copied, the
    left hand side object will continue to be part of the same list
    unless explicitly changed. */
Volume& Volume::operator=(const Volume& v)
{
  if (this == &v)
    return *this;

  deinit();
  init();

  volume = v.volume;
  norm = v.norm;
  zonePtr = v.zonePtr;
  mixPtr = v.mixPtr;
  userVol = v.userVol;
  // Need to confirm desired behavior of assignment operator for this variable
  adjConv = v.adjConv;
  
  if (v.zoneName != NULL)
    {
      zoneName = new char[strlen(v.zoneName)+1];
      memCheck(zoneName,"Volume::operator=(...) : zoneName");
      strcpy(zoneName,v.zoneName);
    }

  return *this;
}

/****************************
 *********** Input **********
 ***************************/

/** It extends the linked list until reaching the keyword "end". */
void Volume::getVolumes(istream& input)
{
  char token[64],name[64];
  Volume *ptr = this;

  verbose(2,"Reading volumes and zone assignments of intervals.");

  /* read list of interval definitions until keyword "end" */
  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      input >> name;
      ptr->next = new Volume(atof(token),name);
      memCheck(next,"Volume::getVolumes(...): next");
      ptr = ptr->next;

      verbose(3,"Added interval with volume %g in zone %s.",atof(token),name);

      clearComment(input);
      input >> token;
    }

}


/***************************
 ********* xCheck **********
 **************************/

/** When intervals are specified in the input file (as opposed to
    being calculated by the code), zones are given in the input. When
    the referenced zone is found, the 'zonePtr' member is set. */
void Volume::xCheck(Loading *loadList)
{
  Volume *ptr=this;

  verbose(2,"Checking for all zones referenced in interval definitions.");

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      ptr->zonePtr = loadList->findZone(ptr->zoneName);
      if (ptr->zonePtr == NULL)
	{
	  error(420,"Zone %s specified in interval volumes was not found in the material loading.", ptr->zoneName);
	}
    }

  verbose(3,"All zones referenced in interval definitions were found.");

}

/****************************
 ********* Preproc **********
 ***************************/

/** The coordinates of the inside corner of the interval are specified
    in the first argument with the dimensions of the interval in the
    second.  The third argument specifies the order in which the first
    two arguments should be interpreted.  The fourth argument is used
    to determine the type of geometry and the fifth for
    cross-referencing the newly generated interval with a zone. */
Volume* Volume::calculate(double *da, double *dx, int *coord, 
			      Geometry* geom, Loading *loadPtr)
{
  double ri,ro,dt,dz,pi,po,dp;

  /* debug(3,"Adding interval at point (%g,%g,%g), with size (%g,%g,%g) and coordinate ordering (%d,%d,%d) in zone %s.",
	da[0],da[1],da[2],dx[0],dx[1],dx[2],coord[0],coord[1],coord[2],loadPtr->getName()); */

  switch (geom->getType())
    {
    case GEOM_R:
      next = new Volume(dx[0]*dx[1]*dx[2],loadPtr);
      memCheck(next,"Volume::calculate(...): next");
      break;
    case GEOM_C:
      ri = da[coord[0]];
      ro = ri + dx[coord[0]];
      dt = dx[coord[1]];
      dz = dx[coord[2]];
      next = new Volume((ro*ro - ri*ri)*dt*dz/2,loadPtr);
      memCheck(next,"Volume::calculate(...): next");
      break;
    case GEOM_S:
      ri = da[coord[0]];
      ro = ri + dx[coord[0]];
      dt = dx[coord[1]];
      pi = da[coord[2]];
      po = pi + dx[coord[2]];
      dp = fabs(cos(po)-cos(pi));
      next = new Volume((ro*ro*ro - ri*ri*ri)*dp*dt/3,loadPtr);
      memCheck(next,"Volume::calculate(...): next");
      break;
    case GEOM_T:
      ri = da[coord[0]];
      ro = ri + dx[coord[0]];
      dp = dx[coord[2]];
      next = new Volume(geom->majR()*(ro*ro-ri*ri)*dp*PI,loadPtr);
      memCheck(next,"Volume::calculate(...): next");
      break;
    }

  verbose(3,"Added interval with volume %g in zone %s.",next->volume, next->zoneName);

  return next;
     
}

/** This is technically redundant once the zone's have been
    cross-referenced, but is prevents too many look-ups later in the
    code.  The 'mixPtr' pointer is set following a lookup on the list
    pointed to by the first argument.  The 'this' pointer is then
    passed to the object pointed to by 'mixPtr' to reference it the
    other way. */
void Volume::xRef(Mixture *mixListHead)
{

  Volume *ptr = this;

  verbose(2,"Cross-referencing intervals with mixtures.");
  
  /* for each interval */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (strcmp(ptr->zonePtr->getMix(),"void"))
	{
	  /* set the mixture pointer based on the zone pointer */
	  ptr->mixPtr = mixListHead->find(ptr->zonePtr->getMix());
	  /* add this interval to the mixture's list of intervals */
	  ptr->mixPtr->xRef(ptr);
	  ptr->nComps = ptr->mixPtr->getNComps();
	  ptr->outputList = new Result[ptr->nComps+1];
	  verbose(3,"Referencing mixture from zone %s (%s) -> %s.",
		  ptr->zonePtr->getName(), ptr->zonePtr->getMix(),
		  ptr->mixPtr->getName());
	}
    }
}

void Volume::xRef(Norm *normList)
{
  Volume *ptr=this;
  int cntNorms=0;

  verbose(2,"Adding spatial normalization to intervals.");

  while (ptr->next != NULL)
    {
      ptr=ptr->next;
      normList = normList->advance();
      if (normList == NULL)
	error(620,"You have specified too few normalizations.  If you specifiy any normalizations, you must specify one for each interval.");

      ptr->norm = normList->getScale();
      cntNorms++;
    }

  if (normList->advance() != NULL)
    warning(621,"You have specified too many normalizations.  Extra normalizations will be ignored.");
    
  verbose(3,"Added %d spatial normalizations to intervals.",cntNorms);

}

/** Volume::xRef(Mixture*) passes its 'this' pointer to a Mixture
    object which immediately passes it back to a different Volume
    object, that mixture's volList pointer, as the base of this call. */
void Volume::addMixList(Volume* ptr)
{
  Volume *volList = this;

  while (volList->mixNext != NULL)
    volList = volList->mixNext;

  volList->mixNext = ptr;
}


// THIS FUNCTION IS DEPRECATED SINCE ADDING SUPPORT FOR RTFLUX FILES (09/2002)
/* read the appropriate flux into each interval's flux member object */
/* called by Flux::xRef(...) */
// void Volume::readFlux(char* fname, int skip, double scale)
// {
//   Volume* ptr = this;
//   ifstream fluxFile(fname);
//   int skipNum;
//   int nGroups = VolFlux::getNumGroups();
//   double skipDble;
  
//   debug(3,"Reading flux in %d groups from %s for all intervals",nGroups,fname);

//   /* skip entries at beginning of file */
//   if (skip>0)
//     for (skipNum=0;skipNum<skip*nGroups;skipNum++)
//       fluxFile >> skipDble;

//   if (fluxFile.eof())
//     error(622,"Flux file %s does not contain enough data.",fname);

//   /* read the flux for each interval */
//   while (ptr->next != NULL)
//     {
//       if (fluxFile.eof())
// 	error(622,"Flux file %s does not contain enough data.",fname);

//       ptr = ptr->next;
//       ptr->flux = ptr->flux->read(fluxFile,scale*ptr->norm);
//     }

// }

/** It scales the values by the third argument, and skips some values
    according to the second argument. */
void Volume::storeMatrix(double** fluxMatrix, double scale)
{
  Volume *ptr = this;
  int VolNum = 0;

  while(ptr->next)
    {
      ptr = ptr->next;
      ptr->flux = ptr->flux->copyData(fluxMatrix[VolNum],scale*ptr->norm);
      VolNum++;
    }
}



/** The first argument points to the problem schedule
    hierarchy so that the correct storage space hierarchy can be
    initialized. */
void Volume::makeSchedTs(topSchedule *top)
{
  Volume* ptr = this;

  verbose(2,"Making storage hierarchies in intervals.");
  
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (ptr->mixPtr != NULL)
	{
	  ptr->schedT = new topScheduleT(top);
	  memCheck(ptr->schedT,"Volume::makeSchedTs(...): ptr->schedT");
	  verbose(6,"Made next storage hierarchy.");
	}
      else
	verbose(6,"Skipped storage hierarchy in VOID interval.");
    }

  verbose(3,"Made all storage hierarchies.");
}

/*****************************
 ********* Solution **********
 ****************************/

/** For each interval in the mixture list, the reference flux (part of
    the Volume pointed to by the first argument) is compared with the
    interval's flux and updated if necessary. */
void Volume::refFlux(Volume *refVolume)
{
  Volume* ptr= this;
  
  while (ptr->mixNext != NULL)
    {
      ptr = ptr->mixNext;

      /* tally volume weights */
      refVolume->volume += ptr->volume;

      /* collapse the rates with the flux */
      refVolume->fluxHead->updateReference(ptr->fluxHead,ptr->volume);
    }


}

/** Argument 1 is the chain, and agument 2 is the master schedule.
    The chain is folded with the fluxes to get scalar rates, the
    transfer matrices are set (which generates the solution), and the
    results are tallied. */
void Volume::solve(Chain* chain, topSchedule* schedule)
{
  Volume* ptr= this;
  
  while (ptr->mixNext != NULL)
    {
      ptr = ptr->mixNext;
      /* collapse the rates with the flux */
      chain->collapseRates(ptr->fluxHead);
      /* solve the schedule */
      schedule->setT(chain,ptr->schedT);
      /* tally results */
      ptr->results.tallySoln(chain,ptr->schedT);

    }  
}

/** It calls writeDump() for each interval in the mixture's list of
    intervals. */
void Volume::writeDump()
{
  Volume* ptr= this;
  
  while (ptr->mixNext != NULL)
    {
      ptr = ptr->mixNext;
  
      ptr->results.writeDump();

    }
}

/** Instead, the topSchedule storage pointer 'schedT' is returned in
    order to extract an array of relative productions at the various
    cooling times. */
topScheduleT* Volume::solveRef(Chain* chain, topSchedule* schedule)
{
  /* collapse the rates with the flux */
  chain->collapseRates(fluxHead);
  /* solve the schedule */
  schedule->setT(chain,schedT);

  return schedT;

}

/*****************************
 ********* PostProc **********
 ****************************/

/** It calls readDump() for each interval in this mixture's list of
    intervals, and then tallies those results to the various lists of
    results. */
void Volume::readDump(int kza)
{
  Volume* ptr= this;
  int compNum;
  
  while (ptr->mixNext != NULL)
    {
      ptr = ptr->mixNext;

      /* read the data from the binary dump */
      ptr->results.readDump();

      switch(NuclearData::getMode())
	{
	case MODE_FORWARD:
	  {
	    /* tally the results from this root in the various components */
	    ptr->results.postProcList(ptr->outputList,ptr->mixPtr,kza);
	    break;
	  }
	case MODE_REVERSE:
	  {
	    /* tally the results from this target in various components */
	    ptr->results.postProcTarget(ptr->outputList,ptr->mixPtr);
	  }
	}
    }
}

void Volume::postProc()
{
  Volume *ptr = this;
  int compNum, intvlCntr=0;

  verbose(2,"Tallying constituent results into total result lists.");
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
 
      if (ptr->mixPtr != NULL)
	{
	  verbose(3,"Tallying for interval #%d",++intvlCntr);
	  /* tally each of the components into the total */
	  for (compNum=0;compNum<ptr->nComps;compNum++)
	    ptr->outputList[compNum].postProc(ptr->outputList[ptr->nComps]);
	  
	  /* tally the results to the respective mixture and zone */
	  verbose(3,"Tallying interval #%d into mixture %s",++intvlCntr,
		  ptr->mixPtr->getName());
	  ptr->mixPtr->tally(ptr->outputList,ptr->volume);
	  verbose(3,"Tallying interval #%d into zone %s",++intvlCntr,
		  ptr->zoneName);
	  ptr->zonePtr->tally(ptr->outputList,ptr->volume);
	}
      else
	verbose(3,"Skipping VOID interval #%d.",++intvlCntr);
    }
  ptr=this;
  while (ptr->next !=NULL)
  {
	ptr=ptr->next;
	if (ptr->mixPtr != NULL)
        {	ptr->userVol=(ptr->zonePtr->getUserVol())*(ptr->volume)/(ptr->zonePtr->getVol());
 		ptr->mixPtr->incrUserVol(ptr->userVol);
	} 
  }    
}


/** The first argument indicates which kind of response is being
    written, the second indicates whether a mixture component breakdown
    was requested, and the third points to the list of after-shutdown
    cooling times.  The fourth argument indicates the kza of the target
    isotope for a reverse calculation and is simply passed on the the
    Result::write().  The final argument indicates what type of
    normalization is being used, so that the correct output information
    can be given. */
void Volume::write(int response, int writeComp, CoolingTime* coolList, 
		   int targetKza, int normType)
{
  Volume *head = this;
  Volume *ptr = head;
  int intvlCntr = 0;
  double volFrac, volume_mass, density;

  /* for each interval */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      /* write header information */
      cout << endl;
      cout << "Interval #" << ++intvlCntr << " (Zone: " 
	   << ptr->zoneName <<") :" << endl;

      debug(5,"Volume::userVol= %f",ptr->userVol);
      if (ptr->mixPtr != NULL)
	{
	  if (normType > 0)
	    cout << "\tRelative Volume: " << ptr->volume << endl;
	  else
	    cout << "\tMass: " << ptr->volume*ptr->mixPtr->getTotalDensity() 
		 << endl;

	  cout << "\tContaining mixture: " << ptr->mixPtr->getName() << endl 
	       << endl;
	  
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
		  /* NOTE: interval results are already normalized 
		           for the correct total interval volume */
		  volFrac = compPtr->getVolFrac();
		  volume_mass = volFrac;

		  /* write component header */
		  cout << "Constituent: " << compPtr->getName() << endl;

		  if (normType < 0)
		    {
		      density = compPtr->getDensity();
		      cout 
		        << "\tVolume Fraction: " << volFrac
		        << "\tRelative Volume: " << volume_mass*ptr->volume;
		      volume_mass *= density;
		      cout
			<< "\tDensity: " << density 
			<< "\tMass: " << volume_mass*ptr->volume;
		    }
		  else if (normType == OUTNORM_VOL_INT) 
		    {
		      /* this effectively multiplies by the interval volume to give
			 a volume integrated result */
		      cout 
		        << "\tVolume Fraction: " << volFrac
		        << "\tAbsolute Volume: " << volume_mass*ptr->userVol;
		      volume_mass /= ptr->userVol;
		      cout << "\tVolume Integrated";
		    }

		  cout << endl;

		  ptr->outputList[compNum].write(response,targetKza,ptr->mixPtr,
						 coolList,ptr->total,volume_mass,ptr);

		  compPtr = compPtr->advance();
		  compNum++;
		}
	    }
	  
	  volFrac = ptr->mixPtr->getVolFrac();

	  /* if components were written and there is only one */
	  if (writeComp && ptr->nComps == 0 && volFrac == 1.0)
	    /* write comment refering total to component total */
	    cout << "** Interval totals are the same as those of the single constituent."
		 << endl << endl;
	  else
	    {
	      /* otherwise write the total response for the zone */
	      volume_mass = volFrac;
	      
	      /* write component header */
	      cout << "Total (All constituents) " << endl;

	      cout << "\tCOMPACTED" << endl;

	      
	      if (normType < 0)
		{
	          cout 
		    << "\tVolume Fraction: " << volFrac
		    << "\tRelative Volume: " << volume_mass*ptr->volume;
		  density = ptr->mixPtr->getTotalDensity();
		  /* different from constituent: mixture densities 
		     already take volume fraction into account */
		  volume_mass = density;
		  cout
		    << "\tDensity: " << density 
		    << "\tMass: " << volume_mass*ptr->volume;
		}
	      else if (normType == OUTNORM_VOL_INT) 
		{
		  /* this effectively multiplies by the interval volume to give
		     a volume integrated result */
	          cout 
		    << "\tVolume Fraction: " << volFrac
		    << "\tAbsolute Volume: " << volume_mass*ptr->userVol;
		  cout << "\tVolume Integrated";
		  volume_mass /= ptr->userVol;
		}
	      cout << endl;

	      ptr->outputList[ptr->nComps].write(response,targetKza,ptr->mixPtr,
						 coolList,ptr->total, volume_mass,ptr);
	      
	    }
	}
      else
	cout << "\tMixture: VOID" << endl << endl;
      
    }
	  

  /** WRITE TOTAL TABLE **/
  /* reset interval pointer and counter */
  ptr = head;
  intvlCntr = 0;

  int resNum,nResults = topScheduleT::getNumCoolingTimes()+1;
  char isoSym[15];

  cout << endl;
  cout << "Totals for all intervals." << endl;

  /* write header for totals */
  coolList->writeTotalHeader("interval");

  /* for each interval */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      intvlCntr++;
      if (ptr->mixPtr != NULL)
	{
	  cout << intvlCntr << "\t";
	  for (resNum=0;resNum<nResults;resNum++)
	    {
	      sprintf(isoSym,"%-11.4e ",ptr->total[resNum]);
	      cout << isoSym;
	    }
	  cout << endl;
	}
    }
  coolList->writeSeparator();

  cout << endl << endl;
}


/** It does this because the results are not cummulative across subsequent
    targets. */
void Volume::resetOutList()
{
  int compNum;

  Volume *ptr = this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (ptr->outputList != NULL)
	for (compNum=0;compNum<=ptr->nComps;compNum++)
	  ptr->outputList[compNum].clear();
    }
}

int Volume::count()
{
  int numInt = 0;
  Volume* ptr = this;
  
  while(ptr->next)
  {
    ptr = ptr->next;
    numInt++;
  }

  return numInt;
}

void Volume::makeXFlux(Mixture *mixListHead)
{
  // Get data from VolFlux
  int nNEG  = VolFlux::getNumGroups();
  int nCP   = VolFlux::getNumCP();
  int nCPEG = VolFlux::getNumCPEG();
  
  Volume *ptr=this;
  VolFlux *volFluxPtr=NULL;
  Mixture* mixPtr = mixListHead;

  // Calculate Mixture ranges and Gvalues
  while(mixPtr->getNext())
    {
      mixPtr = mixPtr->getNext();
      mixPtr->calcGvalues();
    }

  // Calculate Charged Partical Spectrum
  double sumNEG = 0;
  while(ptr->next)
    {
      ptr = ptr->next;
      volFluxPtr = ptr->fluxHead;
      while(volFluxPtr->advance())
	{
	  volFluxPtr = volFluxPtr->advance();
	  
	  for(int CP = 0; CP < nCP; CP++)
	    {
	      for(int CPEG = 0; CPEG < nCPEG; CPEG++)
		{
		  sumNEG = 0;
		  for(int NEG = 0; NEG < nNEG; NEG++)
		    {
		      sumNEG += volFluxPtr->getnflux()[NEG]*ptr->mixPtr->getGvalues()[CP][CPEG][NEG];
		    }
		  volFluxPtr->setCPflux(CP,CPEG,sumNEG);
		  //cout << volFluxPtr->getCPflux()[CP][CPEG] << ' ';
		}
	      //cout << endl << endl;
	    }
	  //cout << endl << endl << "Next CP" << endl << endl;
	}
    }
	
}

void Volume::loadSpecLib(istream *probInput)
{
  // Get data from VolFlux class
  int nNEG  = VolFlux::getNumGroups();
  int nCP   = VolFlux::getNumCP();
  int nCPEG = VolFlux::getNumCPEG();

  energyRel = new int[nNEG];

  char fileName[100];
  *probInput >> fileName;

  int KZA;
  int count = 0;
  char String[100];
  int Start,Finish;
  ifstream input(fileName);
  double *specLibStorage = 0;

  if(!input.is_open())
    cout << "\nFailed to open spec lib\n";

  // Find Energy Group Conversions
  if(energyRel)
    {
      input.getline(String,100,'\n');
      input.getline(String,100,'\n');

      for(int i = 0; i < 20; i++)
	{
	  input >> Start >> Start >> Start;
	  
	  if(Start/1000)
	    {
	      Finish = Start % 1000;
	      Start = Start/1000;
	    }
	  else
	    {
	      input >> Finish;
	    }

	  for(int j = Start; j <= Finish; j++)
	    energyRel[j-1] = i;	    

	  for(int j = 0; j < 21; j++)
	    input.getline(String, 100, '\n');
	}

      input.seekg(0,ios::beg);
    }

  // Get Spec Data
  while(!input.eof())
    {
      input >> KZA;
      input.getline(String,100,'\n');
      specLibStorage = new double[2400];

      for(int i = 0; i < 20; i++)
        {
          input.getline(String,100,'\n');

          for(int j=0; j < nCP; j++)
            {
              input.getline(String,100,'\n');

              for(int k = 0; k < nCPEG; k++)
                {
                  input >> specLibStorage[count];
                  count++;
                }
	      
              input.getline(String,100,'\n');
            }
	  specLib[KZA] = specLibStorage;
        }
      input.get();
      count = 0;
    }  
}

void Volume::loadRangeLib(istream *probInput)
{
  // Get data from VolFlux Class
  int nCP   = VolFlux::getNumCP();
  int nCPEG = VolFlux::getNumCPEG();

  char fileName[30],tmpStr[10],tmpStr2[10];
  tmpStr[0] = '.';
  *probInput >> fileName;

  ifstream *input = new ifstream[nCP];

  for(int i = 0; i < nCP; i++)
    {
      strcpy(tmpStr2,fileName);
      sprintf(&tmpStr[1],"%d",i);
      input[i].open(strcat(tmpStr2,tmpStr));
      
      if(!input[i].is_open())
	{
	  cout << "\nFailed To Open Range Libraries\n";
	}
    }

  // 0: alpha
  // 1: deuteron
  // 2: helium 3
  // 3: proton
  // 4: triton
  
  char String[100];
  int KZA;
  double *rangeStorage;
  
  for(int i = 0; i < nCP; i++)
    {
      while(!input[i].eof())
        {
          input[i] >> KZA;
	  KZA = KZA/10000;

          if(!i)
            {
              rangeStorage = new double[nCP*nCPEG];
              Volume::rangeLib[KZA] = rangeStorage;
            }
          
          input[i].getline(String,100,'\n');
          for(int j = 0; j < nCPEG; j++)
            {
              input[i] >> Volume::rangeLib[KZA][i*24+j];
            }

          input[i].get();
        }
    }

}

void Volume::readAdjDoseData(int nGroups, ifstream& AdjDoseData)
{
  Volume *ptr = this;
  while (ptr->next !=NULL)
  {
	ptr = ptr->next;
        ptr->adjConv=new double[nGroups]; 
        for (int gNum=0;gNum<nGroups;gNum++)
	       AdjDoseData >> ptr->adjConv[gNum];
  }
}

double Volume::getAdjDoseConv(int kza, GammaSrc *adjDose)
{
 
  return adjDose->calcAdjDose(kza,adjConv,userVol);
}
