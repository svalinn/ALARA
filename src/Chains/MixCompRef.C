/* $Id: MixCompRef.C,v 1.7 2002-06-04 18:05:28 wilsonp Exp $ */
/* File sections:
 * Service: constructors, destructors
 * Chain: functions directly related to the building and analysis of chains
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */

#include "Root.h"

#include "Input/Mixture.h"

/****************************
 ********* Service **********
 ***************************/

Root::MixCompRef::MixCompRef(Mixture* addMix, Component* addComp, 
			     double isoDens) :
  mixPtr(addMix), compPtr(addComp), density(isoDens)
{
  next = NULL;
}

Root::MixCompRef::MixCompRef(const Root::MixCompRef& m) :
  mixPtr(m.mixPtr), compPtr(m.compPtr), density(m.density)
{
  next = NULL;
  if (m.next != NULL)
    next = new MixCompRef(*(m.next));
}

Root::MixCompRef::MixCompRef(MixCompRef* cpyRef, MixCompRef* nxtPtr)
{
  mixPtr = cpyRef->mixPtr;
  compPtr = cpyRef->compPtr;
  density = cpyRef->density;
  next = nxtPtr;
}

Root::MixCompRef& Root::MixCompRef::operator=(const Root::MixCompRef& m)
{
  if (this == &m)
    return *this;

  mixPtr = m.mixPtr;
  compPtr = m.compPtr;
  density = m.density;

  delete next;
  next = NULL;
  if (m.next != NULL)
    next = new MixCompRef(*(m.next));

  return *this;

}

/****************************
 ********** List ************
 ***************************/

/* add (at end) or insert (next to same mixture)
 * a new reference to a mixture and component */
void Root::MixCompRef::add(MixCompRef* addRef)
{
  MixCompRef* head = this;
  MixCompRef* prev = NULL;
  MixCompRef* ptr = head;

  /* search for the same mix */
  while (ptr->mixPtr != addRef->mixPtr && ptr->next != NULL)
    {
      prev = ptr;
      ptr = ptr->next;
    }

  /* insert new ref, either at end, or next to same mix */
  ptr->next = new MixCompRef(addRef,ptr->next);
  memCheck(ptr->next,"Root::MixCompRef::add(...): ptr->next");

}

/* tally a list of references to mixtures and components to this root */
void Root::MixCompRef::tally(MixCompRef* tallyList)
{
  MixCompRef *head = this;
  MixCompRef *found;
  
  /* for each reference in new list */
  while (tallyList != NULL)
    {
      /* search for matching mix and component ref */
      found = head->find(tallyList);
      
      /* if not found, add it */
      if (found == NULL)
	head->add(tallyList);

      /* go to next ref */
      tallyList = tallyList->next;
    }
}      

      
/****************************
 ******** Solution **********
 ***************************/

/* setup the reference flux for a particular root isotope */
void Root::MixCompRef::refFlux(Volume *refVolume)
{
  MixCompRef *ptr = this;
  MixCompRef *oldPtr;

  while (ptr != NULL)
    {
      oldPtr = ptr;

      /* solve mixture */
      ptr->mixPtr->refFlux(refVolume);

      /* find next mixture */
      while (ptr != NULL && ptr->mixPtr == oldPtr->mixPtr)
	ptr = ptr->next;
      
    }
  
}



/* solve the chain for each mixture (but not for each component) */
void Root::MixCompRef::solve(Chain *chain, topSchedule* schedule)
{
  MixCompRef *ptr = this;
  MixCompRef *oldPtr;

  while (ptr != NULL)
    {
      oldPtr = ptr;

      /* solve mixture */
      ptr->mixPtr->solve(chain,schedule);

      /* find next mixture */
      while (ptr != NULL && ptr->mixPtr == oldPtr->mixPtr)
	ptr = ptr->next;
      
    }
  
}


void Root::MixCompRef::writeDump()
{

  MixCompRef *ptr = this;
  MixCompRef *oldPtr;

  while (ptr != NULL)
    {
      oldPtr = ptr;

      /* solve mixture */
      ptr->mixPtr->writeDump();

      /* find next mixture */
      while (ptr != NULL && ptr->mixPtr == oldPtr->mixPtr)
	ptr = ptr->next;
      
    }
  
}

/****************************
 ******** Utility ***********
 ***************************/

/* find an identical reference, or return NULL */
Root::MixCompRef* Root::MixCompRef::find(MixCompRef* srchRef)
{
  MixCompRef *ptr = this;

  while (ptr != NULL)
    {
      /* break if this reference matches the mix AND the component */
      if (ptr->mixPtr == srchRef->mixPtr &&
	  ptr->compPtr == srchRef->compPtr)
	break;
      ptr = ptr->next;
    }

  return ptr;
}

/* find an identical reference, or return NULL */
Root::MixCompRef* Root::MixCompRef::find(Mixture* mix, Component* comp)
{
  MixCompRef *ptr = this;

  while (ptr != NULL)
    {
      /* break if this reference matches the mix AND the component */
      if (ptr->mixPtr == mix)
	if (ptr->compPtr == comp || comp == NULL)
	  break;
      ptr = ptr->next;
    }

  return ptr;
}

/* search list of mixtures and find maximum relative concentration */
double Root::MixCompRef::maxConc()
{
  double relConc,maxRelConc = -1;

  MixCompRef *ptr = this;
  
  while (ptr != NULL)
    {
      maxRelConc = std::max(maxRelConc,ptr->density/ptr->mixPtr->getTotalNDensity());
      ptr = ptr->next;
    }

  return maxRelConc;
}

/* search list of mixtures and find total density across all components */
double Root::MixCompRef::mixConc(Mixture* currMixPtr)
{
  double totalDens = 0;

  MixCompRef *ptr = this;

  while (ptr != NULL)
    {
      if (ptr->mixPtr == currMixPtr)
	totalDens += ptr->density;
      ptr = ptr->next;
    }

  return totalDens;
}

/*****************************
 ********* PostProc **********
 ****************************/

void Root::MixCompRef::readDump(int kza)
{

  MixCompRef *ptr = this;
  MixCompRef *oldPtr;

  while (ptr != NULL)
    {
      oldPtr = ptr;

      /* solve mixture */
      ptr->mixPtr->readDump(kza);

      /* find next mixture */
      while (ptr != NULL && ptr->mixPtr == oldPtr->mixPtr)
	ptr = ptr->next;
      
    }
  
}

Component* Root::MixCompRef::getComp(double& dens, Mixture *mix, Component *lastComp)
{
 
  MixCompRef *ptr = this->find(mix,lastComp);

  /* if no match */
  if (ptr == NULL)
    {
      dens = 0;
      return NULL;
    }
  else
    /* if match, and this is not the first search */
    if (lastComp != NULL)
      /* if there are no more matches for this mixture */
      if (ptr->next == NULL || ptr->next->mixPtr != mix)
	{
	  dens = 0;
	  return NULL;
	}
      else
	/* otherwise advance ptr */
	ptr = ptr->next;

  dens = ptr->density;
  return ptr->compPtr;

}
