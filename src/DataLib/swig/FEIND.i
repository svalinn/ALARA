%module FEIND
%{
#include "../FeindNs.h"
#include "../Consts.h"
#include "../LibDefine.h"
#include "../RamLib.h"
#include <string>
#include <vector>
%}

%include stl.i

namespace std {
        %template(StringVector) vector < std::string >;
};

namespace FEIND
{
   %include ../Consts.h
   %include ../Typedefs.h

   class LibDefine
   {
    public:
      std::vector<std::string> Args;
      FEIND::FEINDFormat Format;
   };

   void LoadLibrary(const LibDefine& lib);

  class RamLib
  {
  public:
    double GetTotalDecayEnergy(Kza parent);
    double GetDecayEnergy(Kza parent, int enType);
    double GetDecayConstant(Kza parent);
  };

  extern RamLib Library;
};
