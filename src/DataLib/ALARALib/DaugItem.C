/* $Id: DaugItem.C,v 1.5 2003-01-06 20:12:07 wilsonp Exp $ */
#include "ADJLib.h"

/** Use member initialization for most members, including NULL values
    for pointers.  Allocate new storage for first parent/reaction */
ADJLib::DaugItem::DaugItem(int addKza, int parKza, long rxnOffset) :
  kza(addKza), next(0), current(0)
{
  parList = new ParItem(parKza,rxnOffset);

}

/** Use member initialization like a memberwise copy constructor */
ADJLib::DaugItem::DaugItem(DaugItem *cpyPtr) :
  kza(cpyPtr->kza),parList(cpyPtr->parList), next(cpyPtr->next), current(0)
{}

/** Create a new list item for a new daughter isotope, but also include the information
    for the first parent/reaction. */
void ADJLib::DaugItem::add(int addKza, int parKza, long rxnOffset)
{

  DaugItem *ptr = this;
  DaugItem *prev = this;

  /* search list for correct location ordered by kza */
  while (ptr != NULL && ptr->kza < addKza)
    {
      prev = ptr;
      ptr = ptr->next;
    }

  if (ptr == NULL)
    {
      /* at end of list */
      prev->next = new DaugItem(addKza,parKza,rxnOffset);
      ptr = prev->next;
    }
  else if (ptr->kza > addKza)
    {
      /* in middle of list */
      ptr->next = new DaugItem(ptr);
      ptr->kza = addKza;
      ptr->parList = new ParItem(parKza,rxnOffset);
      ptr->current = NULL;
    }
  else
    {
      /* if this daughter already exists, just add to the parent/reaction list */
      ptr->parList->add(parKza,rxnOffset);
    }
}

int ADJLib::DaugItem::count()
{

  DaugItem *ptr = this;
  int cntr = 0;
  
  while (ptr != NULL)
    {
      cntr++;
      ptr = ptr->next;
    }

  return cntr;
}

/** This function is based on the where the ADJLib::DaugItem::current member is pointing.
    It assumes that if it is pointing to NULL, that it can be reinitialized at the top
    of the list.  Otherwise, it just advances to the next parent, and if non-NULL, it
    sets the passed-by-reference argument and returns the reaction offset. */
long ADJLib::DaugItem::getNextReaction(int& parKza)
{
  long rxnOffset = 0;

  if (current == NULL)
    current = parList;
  else
    current = current->advance();

  if (current != NULL)
    {
      rxnOffset = current->getOffset();
      parKza = current->getKza();
    }

  return rxnOffset;
}

  
