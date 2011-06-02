#include "XSec.h"
#include "exception/ExInclude.h"

#include <iostream>
#include <cassert>

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

XSec::XSec(unsigned numGroups, double initValue) :
  PCs(NULL)
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

double& XSec::operator[](unsigned i) throw(ExEmptyXSec)
{
  if(!PCs)
    {
      throw(ExEmptyXSec("XSec::operator[]"));
    }
 
  Copy();
  return (*const_cast<vector<double>*>(PCs->P))[i];
}

const double& XSec::operator[](unsigned i) const throw(ExEmptyXSec)
{
  if(!PCs)
    {
      throw(ExEmptyXSec("XSec::operator[] const"));
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

double XSec::Integrate(const vector<double>& mult) const 
  throw(ExXsecSize, ExEmptyXSec)
{
  if(!PCs)
    {
      throw(ExEmptyXSec("XSec::Integrate(...) const"));
    }

  unsigned ng = NumGroups();
  double ret = 0;

  if(ng != mult.size())
    throw ExXsecSize("XSec::Integrate(...) function", ng, mult.size());

  for(unsigned int i = 0; i < ng; i++)
    {
      ret += (*this)[i]*mult[i];
    }

  return ret;
}

XSec& XSec::operator+=(const XSec& rhs) throw(ExXsecSize, ExEmptyXSec)
{
  if(!PCs)
    {
      throw(ExEmptyXSec("XSec::Integrate(...) const"));
    }

  unsigned ng = NumGroups();

  if(ng != rhs.NumGroups())
    throw ExXsecSize("XSec::operator+=(...) function", ng, rhs.NumGroups());

  for(unsigned int i = 0; i < ng; i++)
    {
      (*this)[i] += rhs[i];
    }

  return *this;
}

XSec& XSec::operator*=(double mult) throw(ExEmptyXSec)
{
  if(!PCs)
    {
      throw(ExEmptyXSec("XSec::Integrate(...) const"));
    }

  unsigned ng = NumGroups();

  for(unsigned int i = 0; i < ng; i++)
    {
      (*this)[i] *= mult;
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
      PCs->Count--;
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

ostream& operator<<(ostream& os, XSec rhs) 
{
  if(!rhs.NumGroups()) return os;

  cout << "NUM GROUPS = " << rhs.NumGroups() << endl;

  for(unsigned int i = 0; i < rhs.NumGroups(); i++)
    {
      os << i << "\t" << rhs[i] << endl;
    }

  return os;
}
