/* $Id: ParItem.C,v 1.5 2003-01-06 20:12:07 wilsonp Exp $ */
#include "ADJLib.h"

/** Default value of ParItem::next is NULL indicating end of list */
ADJLib::DaugItem::ParItem::ParItem(const int parKza, const long rxnOffset) :
  kza(parKza), offset(rxnOffset), next(0)
{}

/** Simply copy all the members of the object pointed to by the argument */
ADJLib::DaugItem::ParItem::ParItem(const ParItem* cpyPtr) :
  kza(cpyPtr->kza), offset(cpyPtr->offset), next(cpyPtr->next)
{}

/** Another parent and the binary library offset for the reaction from that parent */
void ADJLib::DaugItem::ParItem::add(const int parKza, const long rxnOffset)
{
  ParItem *ptr = this;
  ParItem *prev = this;

  /* search for correct location ordered by kza */
  while (ptr != NULL && ptr->kza < parKza)
    {
      prev = ptr;
      ptr = ptr->next;
    }

  if (ptr == NULL)
    /* at end of list (and maybe also beginning) */
    prev->next = new ParItem(parKza,rxnOffset);
  else if ( ptr->kza > parKza)
    {
      /* in middle of list (and maybe also beginning) */
      ptr->next = new ParItem(ptr);
      ptr->kza = parKza;
      ptr->offset = rxnOffset;
    }
}

int ADJLib::DaugItem::ParItem::count()
{

  ParItem *ptr = this;
  int cntr = 0;
  
  while (ptr != NULL)
    {
      cntr++;
      ptr = ptr->next;
    }

  return cntr;
}


