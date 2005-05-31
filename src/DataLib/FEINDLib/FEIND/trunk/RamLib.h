#ifndef __RAMLIB_H
#define __RAMLIB_H

#include <map>
#include <vector>
#include <utility>

#include "FeindNs.h"
#include "Parent.h"

class FEIND::RamLib
{
 public:
  /// Return a list of all of the parents in the library
  std::vector<Kza> Parents();

  ErrCode AddSec(const Kza parent, const Kza daughter, const int csType, 
		 const std::vector<double>& cs)
  {
    Data[parent].Secondary[daughter].CrossSections[csType] = cs;
    return FEC_NO_ERROR;
  }

  std::vector<double>  GetSec(const Kza parent, const Kza daughter,
			      const int csType)
  {
    return Data[parent].Secondary[daughter].CrossSections[csType];
  }

  /// Add a parent cross section.
  ErrCode AddPCs(const Kza parent, const int csType, 
		 const std::vector<double>& cs);
  
  /// Add a spectrum.
  ErrCode AddSpectrum(const Kza parent, const int specType, 
		      const Spectrum& spec);
  
  ErrCode AddDecayConstant(Kza parent, const double constant);
/*   ErrCode AddBranchingRatio(const Kza parent, const Kza daughter, */
/* 			    const double br); */
  ErrCode AddDecayMode(Kza parent, int decayMode, int dIso, double br);

  ErrCode AddDecayEnergy(const Kza parent, const int enType, 
			 const double energy);

  ErrCode AddPath(const Kza parent, const Kza daughter, const Path& path);
  ErrCode AddSecPath(const Kza parent, const Kza daughter, const Path& path);  

  ErrCode AddDCs(const Kza parent, const Kza daughter, const int csType,
		 const std::vector<double>& cs);



  ErrCode AddFissionYield(const Kza parent, const Kza daughter,
			  const int fissionType, const double yield);


  std::vector<double> GetPCs(Kza parent, int csType);
  std::vector<double> GetDCs(Kza parent, Kza daughter, int csType);

  std::vector<std::pair<double,double> > GetDiscreteSpec(Kza parent, 
							 int specType);
  double GetDecayConstant(Kza parent);
  double GetDecayEnergy(Kza parent, Kza enType);

  std::vector<std::pair<double,double> > 
    GetFissionYield(Kza parent, int fissionType);
  double GetFissionYield(Kza parent, Kza daughter, int fissionType);

  std::vector<Kza> SecDaughters(Kza parent);
  std::vector<Kza> Daughters(Kza parent);
  double GetSfbr(Kza parent) { return Data[parent].Sfbr; }  

  /// LambdaEff calculates reaction rates.
  /** The reaction rate consists of the decay constant of the parent added to
   *  the radiation induced reaction rates:\n
   *    [INSERT EQUATION]
   *  The total reaction rate is returned.
   */
  ErrCode LambdaEff(Kza parent, const std::vector<double>& flux, 
		    double& result);

  ErrCode LambdaEff(Kza parent, Kza daughter, const std::vector<double>& flux, 
		    double& result);

  double GetBratio(Kza parent, Kza daughter);
  double GetSecBratio(Kza parent, Kza sec_daughter);

  double LambdaEff(Kza parent, const std::vector<double>& flux,
		   std::vector<Kza>& priDs, std::vector<double>& priDsLambda,
		   std::vector<Kza>& secDs, std::vector<double>& secDsLambda);


 private:
  std::map<Kza,Parent> Data;
  std::map<int, std::vector<double> > GroupStructs;
};

#endif
