/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "VolFlux.h"

/****************************
 ********* Service **********
 ***************************/

int VolFlux::nFluxes = 0;

VolFlux::VolFlux(int nGrps) : nGroups(nGrps)
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

VolFlux::VolFlux(const VolFlux& v) :
  nGroups(v.nGroups)
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

VolFlux::VolFlux(ifstream &fluxFile, double scale, int nGrps) :
  nGroups(nGrps)
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
  nGroups = v.nGroups;

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

VolFlux* VolFlux::read(int nGrps, ifstream &fluxFile, double scale)
{
  next = new VolFlux(fluxFile,scale,nGrps);
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
	  for (gNum=0;gNum<compFlux->nGroups;gNum++)
	    if (compFlux->flux[gNum]>reference->flux[gNum])
	      reference->flux[gNum] = compFlux->flux[gNum];
	}
    }


}

double VolFlux::fold(int rateGrps, double* rateVec)
{

  int grpNum;
  double rate=0;


  if (rateVec != NULL && rateGrps == nGroups)
    for (grpNum=0;grpNum<nGroups;grpNum++)
      rate += rateVec[grpNum]*flux[grpNum];

  return rate;
}
