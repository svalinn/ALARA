#include "ADJLib.h"

ADJLib::DaugItem::DaugItem(int addKza, DaugItem *nxtPtr)
{

  kza = addKza;
  parList = NULL;

  next = nxtPtr;
  current = NULL;

}

void ADJLib::DaugItem::add(int addKza, int parKza, long rxnOffset)
{

  DaugItem *ptr = this;
  DaugItem *prev = this;

  while (ptr != NULL && ptr->kza < addKza)
    {
      prev = ptr;
      ptr = ptr->next;
    }

  if (ptr == NULL || ptr->kza > addKza)
    {
      prev->next = new DaugItem(addKza,ptr);
      ptr = prev->next;
    }
  
  ptr->parList->add(parKza,rxnOffset);
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

long ADJLib::DaugItem::getNextReaction()
{
  long rxnOffset = 0;

  if (current == NULL)
    current = parList;
  else
    current = current->advance();

  if (current != NULL)
    rxnOffset = current->getOffset();

  return rxnOffset;
}
  
