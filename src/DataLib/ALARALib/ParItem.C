#include "ADJLib.h"

ADJLib::DaugItem::ParItem::ParItem(int parKza, long rxnOffset)
{
  kza = parKza;
  offset = rxnOffset;
  next = NULL;
}

ADJLib::DaugItem::ParItem::ParItem(ParItem* cpyPtr)
{
  kza = cpyPtr->kza;
  offset = cpyPtr->offset;
  next = cpyPtr->next;
}

void ADJLib::DaugItem::ParItem::add(int parKza, long rxnOffset)
{
  ParItem *ptr = this;
  ParItem *prev = this;

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
