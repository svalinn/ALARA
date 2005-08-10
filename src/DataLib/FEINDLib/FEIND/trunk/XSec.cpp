#include "XSec.h"

#include <iostream>

using namespace std;
using namespace FEIND;

XSec::XSec() :
  PCs(NULL)
{
}

XSec::XSec(const XSec& cs) :
  PCs(cs.PCs)
{
  if(PCs) PCs->Count++;
}

XSec::XSec(unsigned numGroups, double initValue)
{
  Reset(numGroups, initValue);
}

void XSec::Reset(unsigned numGroups, double initValue)
{
  CleanUp();

  PCs = new PCsCounter(new vector<double>(numGroups, initValue));
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

double& XSec::operator[](unsigned i)
{
  if(!PCs)
    {
      // EXCEPTION: accessing null pointer!
    }
 
  Copy();
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

unsigned XSec::NumGroups() const
{
  if(PCs)
    {
      return PCs->P->size();
    }

  return 0;
}

double XSec::Integrate(vector<double>& mult) const
{

  // EXCEPTION: PCS NULL
  // EXCEPTION: VECTOR SIZES

  unsigned ng = NumGroups();
  double ret = 0;

  for(int i = 0; i < ng; i++)
    {
      ret += (*this)[i]*mult[i];
    }

  return ret;
}

XSec& XSec::operator+=(const XSec& rhs)
{
  // EXCEPTION: PCS NULL
  // EXCEPTION: XSEC SIZES

  Copy();
  
  unsigned ng = NumGroups();

  for(int i = 0; i < ng; i++)
    {
      (*this)[i] += rhs[i];
    }

  return *this;
}

XSec& XSec::operator*=(double mult)
{
  // EXCEPTION: PCS NULL
  // EXCEPTION: XSEC SIZES

  Copy();

  unsigned ng = NumGroups();

  for(int i = 0; i < ng; i++)
    {
      (*this)[i] += mult;
    }

  return *this;
}

XSec::operator bool() const
{
  if(PCs) return true;
  return false;
}

void XSec::Copy()
{
  if(PCs && PCs->Count > 1)
    {
      P->Count--;
      PCs = new PCsCounter(new vector<double>( (*PCs->P) ));      
    }
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

XSec::~XSec()
{
  CleanUp();
}
