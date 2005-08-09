#include "XSec.h"

#include <iostream>

using namespace std;
using namespace FEIND;

XSec::XSec() :
  PCs(NULL)
{
}

XSec::XSec(const vector<double>* pcs) :
  PCs(new PCsCounter)
{
  PCs->Count = 1;
  PCs->P = pcs;
}

XSec::XSec(const XSec& cs) :
  PCs(cs.PCs)
{
  if(PCs) PCs->Count++;
}

const XSec& XSec::operator=(const vector<double>* pcs)
{
  if(PCs->P != pcs)
    {
      CleanUp();
      PCs = new PCsCounter;
      PCs->P = pcs;
      PCs->Count = 1;
    }
}

const XSec& XSec::operator=(const XSec& cs)
{
  if(this != &cs)
    {
      CleanUp();
      PCs = cs.PCs;
      if(PCs) PCs->Count++;
    }

  return *this;
}

void XSec::CleanUp()
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

XSec::operator bool() const
{
  if(PCs) return true;
  return false;
}

double& XSec::operator[](unsigned i)
{
  if(!PCs)
    {
      // EXCEPTION: accessing null pointer!
    }
 
  return (*const_cast<vector<double>*>(PCs->P))[i];
}

const double& XSec::operator[](unsigned i) const
{
  if(!PCs)
    {
      // EXCEPTION: accessing null pointer!
    }

  return (*PCs->P)[i];
}

XSec::~XSec()
{
  CleanUp();
}
