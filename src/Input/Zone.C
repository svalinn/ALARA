/* $Id: Zone.C,v 1.4 2003-01-13 04:35:00 fateneja Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */

#include "Zone.h"
#include "Volume.h"
#include "Loading.h"

/***************************
 ********* Service *********
 **************************/


/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  It does NOT
    copy 'next'. */
Zone& Zone::operator=(const Zone& z)
{
  if (this == &z)
    return *this;

  boundary = z.boundary;
  nInts = z.nInts;

  return *this;

}

/****************************
 *********** Input **********
 ***************************/

/** It returns a pointer to the newly created zone. */
Zone* Zone::addZone(int ints, double bound)
{
  next = new Zone(bound,ints);
  memCheck(next,"Zone::addZone(...): next");

  verbose(3,"boundary: %g with %d intervals",bound,ints);

  return next;
}


/****************************
 ********* Preproc **********
 ***************************/

/** It does this following setup for calculation of interval volumes
    in Dimension::convert(...). Volume::convert(...) actually
    calculates the volume and generates the Volume object.  The
    arguments are, respsectively, the coordinates of the starting
    point, the ordering of the coordinates, the pointers to the
    appropriate zoneLists, the problem geometry, the list of problem
    material loadings and the problem's list of intervals. */
void Zone::convert(double *start, int *coord, Zone **zoneStart, 
		   Geometry *geom, Loading *loadList, Volume *volList)
{
  
  double d1[3],d2[3],dx[3];
  Loading *loadPtr = loadList->advance();
  Zone* zonePtr[3];
  int inNum[3], nmInts[3];

  /* A: initialize zone pointers */
  zonePtr[2] = zoneStart[2]->next;
  /* B: initialize lower bound */
  d1[2] = start[2];

  /* C: while this dimension has more boundaries */
  while (zonePtr[2] != NULL)
    {
      /* D: initialize upper bound */
      d2[2] = zonePtr[2]->boundary;
      /* E: initialize number of intervals */
      nmInts[2] = zonePtr[2]->nInts;
      /* F: initialize boundary increment */
      dx[2] = (d2[2]-d1[2])/nmInts[2];

      /* G: for each interval in this boundary dimension */
      for (inNum[2]=0;inNum[2]<nmInts[2];inNum[2]++)
	{

	  /* A: see comment A above */
	  zonePtr[1] = zoneStart[1]->next;
	  /* B */
	  d1[1] = start[1];
	  
	  /* C */
	  while (zonePtr[1] != NULL)
	    {
	      /* D */
	      d2[1] = zonePtr[1]->boundary;
	      /* E */
	      nmInts[1] = zonePtr[1]->nInts;
	      /* F */
	      dx[1] = (d2[1]-d1[1])/nmInts[1];

	      /* G */
	      for (inNum[1]=0;inNum[1]<nmInts[1];inNum[1]++)
		{
		 
		  /* A */
		  zonePtr[0] = zoneStart[0]->next;
		  /* B */
		  d1[0] = start[0];

		  /* C */
		  while (zonePtr[0] != NULL)
		    {
		      /* D */
		      d2[0] = zonePtr[0]->boundary;
		      /* E */
		      nmInts[0] = zonePtr[0]->nInts;
		      /* F */
		      dx[0] = (d2[0]-d1[0])/nmInts[0];

		      /* G */
		      for (inNum[0]=0;inNum[0]<nmInts[0];inNum[0]++)
			{
			  /* debug(4,"Creating new interval at point (%g,%g,%g), with size (%g,%g,%g) and coordinate ordering (%d,%d,%d) in zone %s.",
				d1[0],d1[1],d1[2],dx[0],dx[1],dx[2],coord[0],coord[1],coord[2],loadPtr->getName()); */
			  /* calculate volume of this interval */
			  volList = volList->calculate(d1,dx,coord,geom,loadPtr);
			  /* H */
			  d1[0] += dx[0];
			}
		      
		      /* I */
		      zonePtr[0] = zonePtr[0]->next;
		      loadPtr = loadPtr->advance();
		      /* J */
		      d1[0] = d2[0];
		      
		    }

		  /* H */
		  d1[1] += dx[1];
		  
		}
	      
	      /* I */
	      zonePtr[1] = zonePtr[1]->next;
	      loadPtr = loadPtr->advance();
	      /* J */
	      d1[1] = d2[1];
	      
	    }
	  
	  /* H: increment lower boundary */
	  d1[2] += dx[2];
	}
	  
      /* I: advance to next boundary */
      zonePtr[2] = zonePtr[2]->next;
      loadPtr = loadPtr->advance();
      /* J: reset lower boundary */
      d1[2] = d2[2];
      
    }

}


/****************************
 ********* Utility **********
 ***************************/

int Zone::numZones()
{
  int numZones = 0;
  Zone *ptr=this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      numZones++;
    }

  return numZones;
}

int Zone::numInts()
{
  int nmInts = 0;
  Zone *ptr=this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      nmInts += nInts;
    }

  return nmInts;
}



