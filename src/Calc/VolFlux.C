/* $Id: VolFlux.C,v 1.12 2002-08-23 20:46:16 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "VolFlux.h"

#include "Chains/Node.h"

/****************************
 ********* Service **********
 ***************************/

int VolFlux::nFluxes = 0;
int VolFlux::nGroups = 0;
int VolFlux::refflux_type = REFFLUX_MAX;

VolFlux::VolFlux()
{
  flux = NULL;

  if (nGroups>0)
    {
      flux = new double[nGroups];
      memCheck(flux,"VolFlux::VolFlux(...) constructor: flux");

      for (int gNum=0;gNum<nGroups;gNum++)
	flux[gNum] = 0;
    }

  next = NULL;
}

VolFlux::VolFlux(const VolFlux& v)
{
  flux = NULL;
  
  if (nGroups>0)
    {
      flux = new double[nGroups];
      memCheck(flux,"VolFlux::VolFlux(...) copy constructor: flux");

      for (int gNum=0;gNum<nGroups;gNum++)
	flux[gNum] = v.flux[gNum];
    }


  next = NULL;
}

VolFlux::VolFlux(ifstream &fluxFile, double scale)
{
  int grpNum;

  flux = NULL;

  if (nGroups>0)
    {
      flux = new double[nGroups];
      memCheck(flux,"VolFlux::VolFlux(...) input constructor: flux");

      for (grpNum=0;grpNum<nGroups;grpNum++)
	{
	  fluxFile >> flux[grpNum];
	  flux[grpNum] *= scale;
     	}
    }

  next = NULL;

}

VolFlux::VolFlux(double* fluxData, double scale)
{
  int grpNum;

  flux = NULL;

  if(nGroups>0)
    {
      flux = new double[nGroups];
      
      for(grpNum=0; grpNum < nGroups; grpNum++)
	{
	  flux[grpNum] = fluxData[grpNum]*scale;
	}
    }

  next = NULL;
}

VolFlux& VolFlux::operator=(const VolFlux& v)
{
  if (this == &v)
    return *this;

  delete flux;
  flux = NULL;
  
  if (nGroups>0)
    {
      flux = new double[nGroups];
      memCheck(flux,"VolFlux::opeartor=(...): flux");

      for (int gNum=0;gNum<nGroups;gNum++)
	flux[gNum] = v.flux[gNum];
    }

  return *this;
}

/****************************
 ********* Input ************
 ***************************/

VolFlux* VolFlux::read(ifstream &fluxFile, double scale)
{
  next = new VolFlux(fluxFile,scale);
  memCheck(next,"VolFlux::read(...): next");

  return next;
}

VolFlux* VolFlux::copyData(double *fluxData, double scale)
{
  next = new VolFlux(fluxData, scale);
  return next;
}

/****************************
 ******** Solution **********
 ***************************/

void VolFlux::updateReference(VolFlux *compFlux, double volWeight)
{
  int gNum;
  VolFlux *reference = this;
  
  while (compFlux->next != NULL)
    {
      compFlux = compFlux->next;
      if (reference->next == NULL)
	{
	  reference->next = new VolFlux(*compFlux);
	  memCheck(reference->next,
		   "VolFlux::updateReference(...): reference->next");
	  reference = reference->next;

	  switch (refflux_type) 
	    {
	    case REFFLUX_VOL_AVG:
	      reference->scale(volWeight);
	      break;
	    case REFFLUX_MAX:
	    default:
	      break;
	    }
	}
      else
	{
	  reference = reference->next;
	  for (gNum=0;gNum<nGroups;gNum++)
	    switch (refflux_type) {
	    case REFFLUX_VOL_AVG:
	      reference->flux[gNum] += compFlux->flux[gNum]*volWeight;
	      break;
	    case REFFLUX_MAX:
	    default:
	      if (compFlux->flux[gNum]>reference->flux[gNum])
		reference->flux[gNum] = compFlux->flux[gNum];
	      break;
	    }
	}
    }


}

void VolFlux::scale(double scaleVal)
{
  int gNum;

  VolFlux *ptr = this;
  
  while (ptr != NULL) {
    for (gNum=0;gNum<nGroups;gNum++)
      ptr->flux[gNum] *= scaleVal;
    ptr = ptr->next;
  }
}


double VolFlux::fold(double* rateVec, Node* nodePtr)
{
  int grpNum;
  int baseKza, pathNum, numPaths;
  double rate=0;

  if (rateVec != NULL)
    {
      nodePtr->getRxnInfo(rateVec,baseKza,pathNum,numPaths);
      rate = cache.read(baseKza,pathNum);

      if (rate < 0)
	{
	  rate = 0;
	  for (grpNum=0;grpNum<nGroups;grpNum++)
	    rate += rateVec[grpNum]*flux[grpNum];
	  cache.set(baseKza,numPaths+1,pathNum,rate);
	}
    }
  
  return rate;
}
