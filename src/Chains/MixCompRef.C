/* $Id: MixCompRef.C,v 1.8 2003-01-13 04:34:51 fateneja Exp $ */
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

/** The default constructor sets the pointers to NULL and the 'density'
    to 0. Otherwise, the mixture pointer, component pointer and
    'density' are set with arguments in that order - 'next' is always
    NULL. */
Root::MixCompRef::MixCompRef(Mixture* addMix, Component* addComp, 
			     double isoDens) :
  mixPtr(addMix), compPtr(addComp), density(isoDens)
{
  next = NULL;
}

/** Copy constructor copies is identical to default constructor.
    Therefore, all the members are set except 'next' = NULL. */
Root::MixCompRef::MixCompRef(const Root::MixCompRef& m) :
  mixPtr(m.mixPtr), compPtr(m.compPtr), density(m.density)
{
  next = NULL;
  if (m.next != NULL)
    next = new MixCompRef(*(m.next));
}

/** The new object copies the values of the object pointed to by the
    first argument, and sets its 'next' pointer as the second
    argument. */
Root::MixCompRef::MixCompRef(MixCompRef* cpyRef, MixCompRef* nxtPtr)
{
  mixPtr = cpyRef->mixPtr;
  compPtr = cpyRef->compPtr;
  density = cpyRef->density;
  next = nxtPtr;
}

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.  The 'next'
    pointer is not changed, so that this object remains a member of
    the same list as it originally was in. */
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

/** If none is found, a new reference is added at the end. Note that
    this always adds a new reference - another function determines
    whether or not a new reference is needed. */
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
/** The list pointed to by the argument is merged into the list
    through which the function is accessed.  Each of the items in
    the new list is searched for in the existing list.  All matches
    are ignored and all non-matches generate a new entry in the
    existing list. */
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

/** Each root isotope has a different reference flux, based on the set of
    intervals which contain that root.  This list of intervals in which the
    solution for a given root should be performed is determined by accessing
    mixture reference through this list. */
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
/** The list of intervals in which the solution for a given root isotope should
    be performed is determined by accessing mixture references through this
    list. The Chain and topSchedule are passed through this function to each of
    the mixtures referenced in this list. */
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

/** Returns a pointer to the match, or NULL if none is found. */
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

/** If the second argument is NULL, it returns the first match of the
    Mixture alone, if there is one. */
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

/** If the third argument is NULL, it will return the first match of the 
    mixture. Once the entry is found, the first reference argument is set equal
    to the density. */
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
