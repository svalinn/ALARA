#ifndef __CS_H__
#define __CS_H__

#include "FeindNs.h"

#include <vector>

class FEIND::CS
{
 public:

  CS();
  explicit CS(std::vector<double>* pcs);
  explicit CS(const CS& cs);
  
  const CS& operator=(std::vector<double>* pcs);
  const CS& operator=(const CS& cs);

  std::vector<double>& operator*() { return *(PCs->P); }
  std::vector<double>* operator->() { return PCs->P; }

  ~CS();

 private:

  void CleanUp();

  struct PCsCounter
  {
    int Count;
    std::vector<double>* P;
  };

  PCsCounter* PCs;
};

#endif
