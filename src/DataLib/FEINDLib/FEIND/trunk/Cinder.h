#ifndef __CINDER_H
#define __CINDER_H

#include <fstream>
#include <vector>

#include "FeindNs.h"

class FEIND::Cinder
{
 public:
  Cinder(const LibDefine& lib);
  ErrCode LoadLibrary();
  
 private:
  std::ifstream InFile;
  
  void ActivationData();
  void Transmutation();
  void FissionYields();

  Kza ExtractActParent();
  void ExtractFissionParents(int num, std::vector<Kza>& parents,
                             std::vector<char>& fissionType);
  
  bool CheckFission(const std::string& str);
  void SkipMasses(int num);
  std::vector<double> ExtractYield(int num, Kza& daughter);
  Kza CinderToKza(int cinder);
  int FissionType(char t);
  Path Cinder::MakePath(const std::string& str);
  void AddToCs(Kza parent, Kza daughter, std::vector<double>& newCs);

  bool IsPri(Kza parent, Kza daughter) const
    {
      if( ((parent/10)%1000)/2 >= ((daughter/10)%1000))
	return true;

      return false; 
    }

  static unsigned int NumGroups;
};

#endif
