#include "CS.h"

#include <iostream>

using namespace std;
using namespace FEIND;

CS::CS() :
  PCs(NULL)
{
}

CS::CS(vector<double>* pcs) :
  PCs(new PCsCounter)
{
  PCs->Count = 1;
  PCs->P = pcs;
}

CS::CS(const CS& cs) :
  PCs(cs.PCs)
{
  if(PCs) PCs->Count++;
}

const CS& CS::operator=(vector<double>* pcs)
{
  if(PCs->P != pcs)
    {
      CleanUp();
      PCs = new PCsCounter;
      PCs->P = pcs;
      PCs->Count = 1;
    }
}

const CS& CS::operator=(const CS& cs)
{
  if(this != &cs)
    {
      CleanUp();
      PCs = cs.PCs;
      if(PCs) PCs->Count++;
    }

  return *this;
}

void CS::CleanUp()
{
  if(PCs)
    {
      PCs->Count--;

      if(!PCs->Count)
	{
	  delete PCs->P;
	}

      PCs = NULL;
    }
}

CS::~CS()
{
  CleanUp();
}
