/* $Id: VolFlux.C,v 1.7 2000-01-30 06:38:41 wilson Exp $ */
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

/****************************
 ******** Solution **********
 ***************************/

void VolFlux::updateReference(VolFlux *compFlux)
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
	}
      else
	{
	  reference = reference->next;
	  for (gNum=0;gNum<nGroups;gNum++)
	    if (compFlux->flux[gNum]>reference->flux[gNum])
	      reference->flux[gNum] = compFlux->flux[gNum];
	}
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
