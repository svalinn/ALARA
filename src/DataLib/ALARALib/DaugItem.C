#include "ADJLib.h"

ADJLib::DaugItem::DaugItem(int addKza, int parKza, long rxnOffset)
{

  kza = addKza;
  parList = new ParItem(parKza,rxnOffset);
  next = NULL;

  current = NULL;

}

ADJLib::DaugItem::DaugItem(DaugItem *cpyPtr)
{

  kza = cpyPtr->kza;
  parList = cpyPtr->parList;
  next = cpyPtr->next;

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

long ADJLib::DaugItem::getNextReaction(int &parKza)
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
  
