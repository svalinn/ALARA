/* $Id: VolFlux.C,v 1.15 2003-01-13 04:34:28 fateneja Exp $ */
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
int VolFlux::nCP = 0;
int VolFlux::nCPEG = 0;

/** This constructor creates storage for 'flux' if 'nGroups'>0,
    otherwise sets 'flux' to NULL.  Always sets 'next' to NULL. */
VolFlux::VolFlux()
{
  nflux = NULL;

  if (nGroups>0)
    {
      nflux = new double[nGroups+120];
      memCheck(nflux,"VolFlux::VolFlux(...) constructor: flux");

      for (int gNum=0;gNum<nGroups;gNum++)
	nflux[gNum] = 0;

      if(nCP)
	{
	  CPflux = new double*[nCP];
	  CPfluxStorage = new double[nCP*nCPEG];
      
	  for(int i = 0; i < nCP; i++)
	    {
	      CPflux[i] = &CPfluxStorage[i*nCPEG];
	      
	      for(int j = 0; j < nCPEG; j++)
		{
		  CPflux[i][j] = 0;
		}
	    }
	}
      else
	{
	  CPfluxStorage = NULL;
	  CPflux = NULL;
	}
    }

  next = NULL;
}

/** This constructor copies 'flux' on element-by-element basis.  Sets
    next to NULL. */
VolFlux::VolFlux(const VolFlux& v)
{
  nflux = NULL;
  
  if (nGroups>0)
    {
      nflux = new double[nGroups+120];
      memCheck(nflux,"VolFlux::VolFlux(...) copy constructor: flux");

      for (int gNum=0;gNum<nGroups;gNum++)
	{
	  nflux[gNum] = v.nflux[gNum];
	}
      CPflux = new double*[nCP];
      CPfluxStorage = new double[nCP*nCPEG];
      
      for(int i = 0; i < nCP; i++)
	{
	  CPflux[i] = &CPfluxStorage[i*nCPEG];
	  
	  for(int j = 0; j < nCPEG; j++)
	    {
	      CPflux[i][j] = v.CPflux[i][j];
	    }
	}

    }

  next = NULL;
}

VolFlux::VolFlux(ifstream &fluxFile, double scale)
{
  int grpNum;

  nflux = NULL;

  if (nGroups>0)
    {
      nflux = new double[nGroups+120];
      memCheck(nflux,"VolFlux::VolFlux(...) input constructor: flux");

      for (grpNum=0;grpNum<nGroups;grpNum++)
	{
	  fluxFile >> nflux[grpNum];
	  nflux[grpNum] *= scale;
     	}

      if(nCP)
	{
	  CPflux = new double*[nCP];
	  CPfluxStorage = new double[nCP*nCPEG];
	  
	  for(int i = 0; i < nCP; i++)
	    {
	      CPflux[i] = &CPfluxStorage[i*nCPEG];
	      
	      for(int j = 0; j < nCPEG; j++)
		{
		  CPflux[i][j] = 0;
		}
	    }
	}
      else
	{
	  CPfluxStorage = NULL;
	  CPflux = NULL;
	}
      
    }
  
  next = NULL;
  
}

/** It takes the values in the array and stores them in VolFlux::Flux */
VolFlux::VolFlux(double* fluxData, double scale)
{
  int grpNum;
  
  nflux = NULL;
  
  if(nGroups>0)
    {
      nflux = new double[nGroups+120];
      
      for(grpNum=0; grpNum < nGroups; grpNum++)
	{
	  nflux[grpNum] = fluxData[grpNum]*scale;
	}
      
      if(nCP)
	{
	  CPflux = new double*[nCP];
	  CPfluxStorage = new double[nCP*nCPEG];
	  
	  for(int i = 0; i < nCP; i++)
	    {
	      CPflux[i] = &CPfluxStorage[i*nCPEG];
	      
	      for(int j = 0; j < nCPEG; j++)
		{
		  CPflux[i][j] = 0;
		}
	    }
	}
      else
	{
	  CPflux = NULL;
	  CPfluxStorage = NULL;
	}
    }

  next = NULL;
}

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed. */
VolFlux& VolFlux::operator=(const VolFlux& v)
{
  if (this == &v)
    return *this;

  delete nflux;
  nflux = NULL;
  
  if (nGroups>0)
    {
      nflux = new double[nGroups];
      memCheck(nflux,"VolFlux::opeartor=(...): flux");

      for (int gNum=0;gNum<nGroups;gNum++)
	nflux[gNum] = v.nflux[gNum];

      if(nCP)
	{
	  CPflux = new double*[nCP];
	  CPfluxStorage = new double[nCP*nCPEG];
	  
	  for(int i = 0; i < nCP; i++)
	    {
	      CPflux[i] = &CPfluxStorage[i*nCPEG];
	      
	      for(int j = 0; j < nCPEG; j++)
		{
		  CPflux[i][j] = v.CPflux[i][j];
		}
	    }
	}
      else
	{
	  CPfluxStorage = NULL;
	  CPflux = NULL;
	}     
    }
      
  return *this;
}

/****************************
 ********* Input ************
 ***************************/
/** The newly created object is pointed to by the 'next' of the
    object through which the function is called and a pointer to the
    newly created object is returned. */
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
/** There are currently two options:
        1) group-wise maximum flux, or
        2) group-wise volume weighted average flux */
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
	      reference->nflux[gNum] += compFlux->nflux[gNum]*volWeight;
	      break;
	    case REFFLUX_MAX:
	    default:
	      if (compFlux->nflux[gNum]>reference->nflux[gNum])
		reference->nflux[gNum] = compFlux->nflux[gNum];
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
      ptr->nflux[gNum] *= scaleVal;
    ptr = ptr->next;
  }
}

/** The second argument points to the Node object associated with this
    rate vector and will be used to determine the indexing information
    for the RateCache (see RateCache). */
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
	    rate += rateVec[grpNum]*nflux[grpNum];
	  cache.set(baseKza,numPaths+1,pathNum,rate);
	}
    }
  
  return rate;
}
