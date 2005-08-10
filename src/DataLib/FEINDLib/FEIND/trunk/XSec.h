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
  XSec(const XSec& cs);  
  XSec(unsigned numGroups, double initValue = 0.0);

  void Reset(unsigned numGroups, double initValue = 0.0);

  const XSec& operator=(const XSec& cs);

  double& operator[](unsigned i);
  const double& operator[](unsigned i) const;

  unsigned NumGroups() const;

  double Integrate(const std::vector<double>& mult) const;

  XSec& operator+=(const XSec& rhs);
  XSec& operator*=(double mult);

  operator bool() const;

  ~XSec();

private:
  
  void CleanUp();
  void Copy();

  struct PCsCounter
  {
    PCsCounter(const std::vector<double>* p) :
      Count(1),
      P(p)
    {
    }

    int Count;
    const std::vector<double>* P;
  };

  PCsCounter* PCs;
};

#endif
