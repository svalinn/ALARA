/* $Id: Volume.C,v 1.19 2000-02-17 00:13:18 wilson Exp $ */
#include "Volume.h"
#include "Loading.h"
#include "Geometry.h"
#include "Mixture.h"
#include "Component.h"
#include "CoolingTime.h"
#include "Norm.h"

#include "Chains/Chain.h"
#include "Chains/Root.h"

#include "Calc/VolFlux.h"
#include "Calc/topSchedule.h"
#include "Calc/topScheduleT.h"

#include "Output/Result.h"
#include "Output/Output_def.h"

/***************************
 ********* Service *********
 **************************/

void Volume::init()
{
  volume = 1;
  norm = 1;
 
  zoneName = NULL;
  zonePtr = NULL;
  mixPtr = NULL;
  next = NULL;
  mixNext = NULL;

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
  delete [] outputList;
}  

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

Volume::Volume(const Volume& v)
{
  init();

  volume = v.volume;
  norm = v.norm;
  zonePtr = v.zonePtr;
  mixPtr = v.mixPtr;

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

  rootPtr->refFlux(fluxHead);

  schedT = new topScheduleT(top);

}

Volume::~Volume() 
{
  deinit();
  delete next; 
}

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

/****** get a list of interval volumes ********/
/* called by Input::read(...) */
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

/* cross-check interval volumes for defined zones in material loading
 * when found, set the zonePtr to the zone Definition */
/* called by Input::xCheck(...) */
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

/* given the size and order of an interval's dimensions,
 * create an interval object (Volume) based on the geometry type
 * and the material loading of this zone */
/* called by Zone::convert(...) */
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

/* cross-reference intervals with mixture definitions */
/* called by Input::preproc(...) */
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

/* cross-reference intervals with normalizations */
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

/* extend list of intervals in a mixture list */
/* called by Mixture:xRef(...) */
void Volume::addMixList(Volume* ptr)
{
  Volume *volList = this;

  while (volList->mixNext != NULL)
    volList = volList->mixNext;

  volList->mixNext = ptr;
}


/* read the appropriate flux into each interval's flux member object */
/* called by Flux::xRef(...) */
void Volume::readFlux(char* fname, int skip, double scale)
{
  Volume* ptr = this;
  ifstream fluxFile(fname);
  int skipNum;
  int nGroups = VolFlux::getNumGroups();
  double skipDble;
  
  debug(3,"Reading flux in %d groups from %s for all intervals",nGroups,fname);

  /* skip entries at beginning of file */
  if (skip>0)
    for (skipNum=0;skipNum<skip*nGroups;skipNum++)
      fluxFile >> skipDble;

  if (fluxFile.eof())
    error(622,"Flux file %s does not contain enough data.",fname);

  /* read the flux for each interval */
  while (ptr->next != NULL)
    {
      if (fluxFile.eof())
	error(622,"Flux file %s does not contain enough data.",fname);

      ptr = ptr->next;
      ptr->flux = ptr->flux->read(fluxFile,scale*ptr->norm);
    }

}

/* setup the hierarchy of storage for transfer matrices */
/* called by Input::preProc(...) */
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

/* set reference flux */
void Volume::refFlux(VolFlux *refFluxHead)
{
  Volume* ptr= this;
  
  while (ptr->mixNext != NULL)
    {
      ptr = ptr->mixNext;
      /* collapse the rates with the flux */

      refFluxHead->updateReference(ptr->fluxHead);
    }


}

/* solve the chain at a single interval */
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

void Volume::writeDump()
{
  Volume* ptr= this;
  
  while (ptr->mixNext != NULL)
    {
      ptr = ptr->mixNext;
  
      ptr->results.writeDump();

    }
}

/* solve the chain at a single interval */
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
}


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

      if (ptr->mixPtr != NULL)
	{
	  if (normType > 0)
	    cout << "\tVolume: " << ptr->volume << endl;
	  else
	    cout << "\tMass: " << ptr->volume*ptr->mixPtr->getTotalDensity() 
		 << endl;

	  cout << "\tContaining mixture: " << ptr->mixPtr->getName() << endl 
	       << endl;
	  
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
		  /* NOTE: interval results are already normalized 
		           for the correct total interval volume */
		  volFrac = compPtr->getVolFrac();
		  volume_mass = volFrac;

		  /* write component header */
		  cout << "Constituent: " << compPtr->getName() << endl;
		  cout 
		    << "\tVolume Fraction: " << volFrac
		    << "\tVolume: " << volume_mass*ptr->volume;

		  if (normType < 0)
		    {
		      density = compPtr->getDensity();
		      volume_mass *= density;
		      cout
			<< "\tDensity: " << density 
			<< "\tMass: " << volume_mass*ptr->volume;
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
	    cout << "** Interval totals are the same as those of the single constituent."
		 << endl << endl;
	  else
	    {
	      /* otherwise write the total response for the zone */
	      if (response != OUTFMT_WDR)
		volFrac = 1.0;

	      volume_mass = volFrac;
	      
	      /* write component header */
	      cout << "Total (All constituents) " << endl;

	      if (response != OUTFMT_WDR)
		cout << "\tNON-Compacted" << endl;
	      else
		cout << "\tCOMPACTED" << endl;

	      cout 
		<< "\tVolume Fraction: " << volFrac
		<< "\tVolume: " << volume_mass*ptr->volume;
	      
	      if (normType < 0)
		{
		  density = ptr->mixPtr->getTotalDensity();
		  /* different from constituent: mixture densities 
		     already take volume fraction into account */
		  volume_mass = density;
		  cout
		    << "\tDensity: " << density 
		    << "\tMass: " << volume_mass*ptr->volume;
		}
	      cout << endl;

	      ptr->outputList[ptr->nComps].write(response,targetKza,coolList,
						 ptr->total, volume_mass);
	      
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
