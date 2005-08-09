#ifndef __XSEC_H__
#define __XSEC_H__

#include "FeindNs.h"

#include <vector>

/// CS is the class for storing cross-sections in FEIND.
/** Presently, it simply implements a smart point/reference counting system
 *  which can be used to prevent duplication of data in the RamLib. This will
 *  also allow pointers that are used to be exception safe.\n\n
 *
 *  Eventually, the CS class will implement member functions for performing
 *  useful tasks with cross-sections...
 */
class FEIND::XSec
{
 public:

  XSec();
  XSec(const std::vector<double>* pcs);
  XSec(const XSec& cs);
  
  const XSec& operator=(const std::vector<double>* pcs);
  const XSec& operator=(const XSec& cs);

  double& operator[](unsigned i);
  const double& operator[](unsigned i) const;

  operator bool() const;

  const std::vector<double>& operator*() const
    { 
      return *(const_cast<std::vector<double>*>(PCs->P)); 
    }

  std::vector<double>& operator*()
    { 
      return *(const_cast<std::vector<double>*>(PCs->P)); 
    }

  const std::vector<double>* operator->() const
    { 
      return const_cast<std::vector<double>*>(PCs->P);
    }

  std::vector<double>* operator->()
    { 
      return const_cast<std::vector<double>*>(PCs->P);
    }

  ~XSec();

 private:

  void CleanUp();

  struct PCsCounter
  {
    int Count;
    const std::vector<double>* P;
  };

  PCsCounter* PCs;
};

#endif
