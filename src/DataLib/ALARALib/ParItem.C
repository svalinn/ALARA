#include "ADJLib.h"

ADJLib::DaugItem::ParItem::ParItem(int parKza, long rxnOffset, ParItem* nxtPtr)
{
  kza = parKza;
  offset = rxnOffset;
  next = nxtPtr;
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

  if (ptr == NULL || ptr->kza > parKza)
    {
      prev->next = new ParItem(parKza,rxnOffset,ptr);
      ptr = prev->next;
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
