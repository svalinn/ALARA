/* $Id: Result.C,v 1.23 2001-12-06 23:16:41 wilsonp Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "Result.h"
#include "GammaSrc.h"
#include "Output_def.h"

#include "Input/CoolingTime.h"
#include "Input/Mixture.h"

#include "Chains/Chain.h"
#include "Chains/Node.h"

#include "Calc/topScheduleT.h"


/****************************
 ********* Service **********
 ***************************/

int Result::nResults = 0;
FILE* Result::binDump = NULL;
const int Result::delimiter = -1;
double Result::actMult = 1;
double Result::metricMult = 1;
GammaSrc* Result::gammaSrc = NULL;

Result::Result(int setKza, Result* nxtPtr)
{
  int resNum;

  kza = setKza;
  N = NULL;
  if (nResults>0)
    {
      N = new double[nResults];
      for (resNum=0;resNum<nResults;resNum++)
	N[resNum] = 0;
    }
  next = nxtPtr;
}

Result::Result(const Result& r)
{
  int resNum;

  kza = r.kza;
  N = NULL;
  if (nResults>0)
    {
      N = new double[nResults];
      for (resNum=0;resNum<nResults;resNum++)
	N[resNum] = r.N[resNum];
    }
  next = NULL;

}

Result::Result(int setKza,float* floatN)
{
  int resNum;

  kza = setKza;
  N = NULL;
  if (nResults>0)
    {
      N = new double[nResults];
      for (resNum=0;resNum<nResults;resNum++)
	N[resNum] = floatN[resNum];
    }
  next = NULL;
}



Result& Result::operator=(const Result& r)
{
  if (this == &r)
    return *this;

  int resNum;

  kza = r.kza;
  delete N;
  N = NULL;
  if (nResults>0)
    {
      N = new double[nResults];
      for (resNum=0;resNum<nResults;resNum++)
	N[resNum] = r.N[resNum];
    }

  return *this;
}

/*******************************
 *********** Utility ***********
 ******************************/


Result* Result::find(int srchKza)
{
  Result *oldPtr, *ptr = this;
  
  while (ptr->next != NULL)
    {
      oldPtr = ptr;
      ptr = ptr->next;
      if (ptr->kza == srchKza)
	return ptr;
      else if (ptr->kza > srchKza)
	{
	  oldPtr->next = new Result(srchKza,ptr);
	  return oldPtr->next;
	}
    }

  ptr->next = new Result(srchKza);

  return ptr->next;

}


/****************************
 ********** Tally ***********
 ***************************/

void Result::tallySoln(Chain *chain, topScheduleT* schedT)
{
  Result *ptr, *head = this;
  int rank, setKza;

  /* get the rank of the first node to be tallied */
  rank = chain->getSetRank();

  /* get the first kza to be set */
  setKza = chain->getKza(rank);

  /* if we are not done (finish condition: kza == 0) */
  while (setKza > 0)
    {      
      /* tally result */
      double *Nlist = schedT->results(rank);

      head->find(setKza)->tally(Nlist);

      /* this is allocated in topScheduleT::results() */
      delete Nlist;

      /* get next isotope */
      setKza = chain->getKza(++rank);
    }
}

void Result::tally(double* Nlist, double scale)
{
  int resNum;

  for (resNum=0;resNum<nResults;resNum++)
    {
      N[resNum] += Nlist[resNum]*scale;
      /* used during debugging 
      if ( isnan(N[resNum]) || isinf(N[resNum]))
	error(2000,"A negative solution has been encountered, suggesting a possible round off error.  Please notify the code author."); */
    }

	

}



/*****************************
 ********* PostProc **********
 ****************************/

void Result::postProcTarget(Result* outputList, Mixture *mixPtr)
{

  Component *compPtr=NULL;
  double density;
  int compNum;
  Result *root = this;

  /* for each initial isotope that generates this target */
  while (root->next != NULL)
    {
      root = root->next;
      
      /* get the first component number and density for this root */
      compPtr = mixPtr->getComp(root->kza,density,NULL);
      
      /* if we found the component */
      while (compPtr)
	{
	  compNum = mixPtr->getCompNum(compPtr);
	  
	  /* update this component */
	  outputList[compNum].find(root->kza)->tally(root->N,density);
	  
	  /* get the next component */
	  compPtr = mixPtr->getComp(root->kza,density,compPtr);
	}
    }

  /* we are done with this data */
  clear();
}

void Result::postProcList(Result* outputList, Mixture *mixPtr, int rootKza)
{
  Component *compPtr=NULL;
  double density;
  int compNum;
  
  /* get the first component number and density for this root
   * isotope */
  compPtr = mixPtr->getComp(rootKza,density,NULL);
  
  /* if we found the component */
  while (compPtr)
    {
      compNum = mixPtr->getCompNum(compPtr);

      /* update this component */
      postProc(outputList[compNum],density);
      
      /* get the next component */
      compPtr = mixPtr->getComp(rootKza,density,compPtr);
    }
  
  /* we are done with this data now */
  clear();
}

void Result::postProc(Result& outputList, double density)
{
  Result *ptr = this;

  /* for each result isotope */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      /* tally the output list */
      outputList.find(ptr->kza)->tally(ptr->N,density);

    }

}      

void Result::write(int response, int targetKza, Mixture *mixPtr, 
		   CoolingTime *coolList, double*& total, double volume_mass)
{
  int resNum;
  int gGrpNum,nGammaGrps;
  Result* ptr = this;
  double multiplier=1.0;
  double *gammaMult = NULL;
  double *photonSrc = NULL;
  Node dataAccess;
  char isoSym[15];
  int mode = NuclearData::getMode();

  /* initialize the total array */
  delete total;
  total = new double[nResults];
  for (resNum=0;resNum<nResults;resNum++)
    total[resNum] = 0;
  
  if (response == OUTFMT_SRC)
    {
      nGammaGrps = gammaSrc->getNumGrps();
      /* initialize total gamma source array */
      photonSrc = new double[nResults*nGammaGrps];
      for (gGrpNum=0;gGrpNum<nResults*nGammaGrps;gGrpNum++)
	photonSrc[gGrpNum] = 0.0;
    }
  
  /* invert volume_mass */
  volume_mass = metricMult/volume_mass;
  
  /* write a standard header for this table */
  coolList->writeHeader();

  if (mode == MODE_REVERSE)
    {
      /* query the data library through a dummy Node object
       * to get the nuclear data for the multiplier */
      switch(response)
	{
	case OUTFMT_SRC:
	  /* set gamma vector */
	  gammaMult = gammaSrc->getGammaMult(targetKza);
	  /* write activity at same time as gamma source */
	case OUTFMT_ACT:
	  multiplier = dataAccess.getLambda(targetKza)*actMult;
	  break;
	case OUTFMT_HEAT:
	  multiplier = dataAccess.getHeat(targetKza) * EV2J;
	  break;
	case OUTFMT_ALPHA:
	  multiplier = dataAccess.getAlpha(targetKza) * EV2J;
	  break;
	case OUTFMT_BETA:
	  multiplier = dataAccess.getBeta(targetKza) * EV2J;
	  break;
	case OUTFMT_GAMMA:
	  multiplier = dataAccess.getGamma(targetKza) * EV2J;
	  break;
	case OUTFMT_WDR:
	  multiplier = dataAccess.getWDR(targetKza)*actMult;
	  break;
	case OUTFMT_CDOSE:
	  multiplier = mixPtr->getDoseConv(targetKza, gammaSrc) *
	    dataAccess.getLambda(targetKza)*actMult;
	  break;
	default:
	  multiplier = 1.0;
	}
      multiplier *= volume_mass;
    }
  

  /* for each isotope in the table */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      if (mode == MODE_FORWARD)
	{
	  /* query the data library through a dummy Node object
	   * to get the nuclear data for the multiplier */
	  switch(response)
	    {
	    case OUTFMT_SRC:
	      /* set gamma vector */
	      gammaMult = gammaSrc->getGammaMult(ptr->kza);
	      /* write activity at same time as gamma source */
	    case OUTFMT_ACT:
	      multiplier = dataAccess.getLambda(ptr->kza)*actMult;
	      break;
	    case OUTFMT_HEAT:
	      multiplier = dataAccess.getHeat(ptr->kza) * EV2J;
	      break;
	    case OUTFMT_ALPHA:
	      multiplier = dataAccess.getAlpha(ptr->kza) * EV2J;
	      break;
	    case OUTFMT_BETA:
	      multiplier = dataAccess.getBeta(ptr->kza) * EV2J;
	      break;
	    case OUTFMT_GAMMA:
	      multiplier = dataAccess.getGamma(ptr->kza) * EV2J;
	      break;
	    case OUTFMT_WDR:
	      multiplier = dataAccess.getWDR(ptr->kza)*actMult;
	      break;
	    case OUTFMT_CDOSE:
	      multiplier = mixPtr->getDoseConv(ptr->kza, gammaSrc) *
		dataAccess.getLambda(ptr->kza)*actMult;
	      break;
	    default:
	      multiplier = 1.0;
	    }
	  multiplier *= volume_mass;
	}
      
      /* if the multipier is 0 (e.g. stable isotope for activity based
	 responses) skip this isotope */
      if (multiplier == 0)
	continue;

      /* write the formatted output for this isotope */
      cout << isoName(ptr->kza,isoSym) << "\t";
      if (response == OUTFMT_SRC)
	gammaSrc->writeIsoName(isoName(ptr->kza,isoSym));

      for (resNum=0;resNum<nResults;resNum++)
	{
	  sprintf(isoSym,"%-11.4e ",ptr->N[resNum]*multiplier);
	  cout << isoSym;

	  /* gamma source */
	  if (response == OUTFMT_SRC)
	    gammaSrc->writeIsotope(gammaMult,ptr->N[resNum]*multiplier/actMult);

	  /* increment the total */
	  total[resNum] += ptr->N[resNum]*multiplier;
	  if (response == OUTFMT_SRC && gammaMult != NULL)
	    /* accumulate gamma source to total */
	    for (gGrpNum=0;gGrpNum<nGammaGrps;gGrpNum++)
	      photonSrc[resNum*nGammaGrps+gGrpNum] += gammaMult[gGrpNum]*ptr->N[resNum]*multiplier/actMult;
	  
	}
      cout << endl;
    }
  
  /* write the gamma source */
  if (response == OUTFMT_SRC)
    gammaSrc->writeTotal(photonSrc,nResults);
  
  /* write a separator for the table */
  coolList->writeSeparator();

  /* write the formatted output for the total response */
  cout << "total\t";
  for (resNum=0;resNum<nResults;resNum++)
    {
      sprintf(isoSym,"%-11.4e ",total[resNum]);
      cout << isoSym;
    }
  cout << endl;

  delete photonSrc;
}

/*****************************
 ********* PostProc **********
 ****************************/

void Result::initBinDump(char* fname)
{ 
  binDump = fopen(fname,"rb+"); 
  if (!binDump)
    {
      binDump = fopen(fname,"wb+");
      if (!binDump)
	error(240,"Unable to open dump file %s",fname);
    }
};

void Result::dumpHeader()
{
  fwrite(&nResults,SINT,1,binDump);
}

void Result::xCheck()
{
  if (binDump == NULL)
    {
      warning(440,"ALARA now requires a binary dump file.  Openning the default file 'alara.dmp'");
      initBinDump("alara.dmp");
    }
}

void Result::resetBinDump()
{
  fflush(binDump);
  fseek(binDump,0L,SEEK_SET);
  fread(&nResults,SINT,1,binDump);
  verbose(1,"Reset binary dump with %d results per isotope.",nResults);
}

void Result::writeDump()
{
  Result *ptr = this;
  static float *floatN = new float[nResults];
  int resNum;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      fwrite(&(ptr->kza),SINT,1,binDump);
      for (resNum=0;resNum<nResults;resNum++)
	floatN[resNum] = ptr->N[resNum];
      fwrite(floatN,SFLOAT,nResults,binDump);

    }

  fwrite(&delimiter,SINT,1,binDump);
      
  clear();
}

void Result::readDump()
{
  Result *ptr = this;
  int readKza,resNum;

  static float *floatN = new float[nResults];
  
  fread(&readKza,SINT,1,binDump);
  while (readKza != delimiter)
    {
      fread(floatN,SFLOAT,nResults,binDump);
      ptr->next = new Result(readKza,floatN);
      ptr = ptr->next;
      fread(&readKza,SINT,1,binDump);
    }
}

void Result::setNorm(double passedActMult, int normType)
{

  actMult = passedActMult;

  switch (normType) {
  case OUTNORM_M3:
    metricMult = 1.0/CM3_M3;
    break;
  case OUTNORM_KG:
    metricMult = 1.0/G_KG;
    break;
  default:
    metricMult = 1;
  }

}
