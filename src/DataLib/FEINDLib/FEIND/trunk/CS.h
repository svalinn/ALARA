#ifndef __CS_H__
#define __CS_H__

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
class FEIND::CS
{
 public:

  CS();
  explicit CS(const std::vector<double>* pcs);
  explicit CS(const CS& cs);
  
  const CS& operator=(const std::vector<double>* pcs);
  const CS& operator=(const CS& cs);

  std::vector<double>& operator*() const
    { 
      return *(const_cast<std::vector<double>*>(PCs->P)); 
    }

  std::vector<double>* operator->() const
    { 
      return const_cast<std::vector<double>*>(PCs->P);
    }

  ~CS();

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
