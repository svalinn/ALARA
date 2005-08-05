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

  /// Add a parent cross-section to the library.
  /** csType indicates which parent cross-section should be added. Examples
   *  include:\n
   *  /code TOTAL_CS \n
   *        NEUTRON_FISSION_CS \endcode \n
   *  The boolean argument add allows the cross-section to be added to
   *  the current one, if true, or to replace the current cross-section, if add
   *  is false. In either case, if the cross-section does not exist, it will
   *  be automatically created.
   */
  ErrCode SetPCs(const Kza parent, const int csType, 
		 const std::vector<double>& cs, bool add = false, 
		 double mult = 1.0);

  const std::vector<double>& GetPCs(Kza parent, int csType);

  ErrCode SetDCs(const Kza parent, const Kza daughter, const int csType,
		 const std::vector<double>& cs, bool add=false, 
		 double mult = 1.0);

  const std::vector<double>& GetDCs(Kza parent, Kza daughter, int csType);
  

  /// Add a spectrum.
  ErrCode AddSpectrum(const Kza parent, const int specType, 
		      const Spectrum& spec);
  
  ErrCode AddDecayConstant(Kza parent, const double constant);

  /// Add a decay mode to the library
  /** This function will automatically add the appropriate information for
   *  secondary radiation as well
   */
  ErrCode AddDecayMode(Kza parent, int decayMode, int dIso, double br);

  ErrCode AddDecayEnergy(const Kza parent, const int enType, 
			 const double energy);

  ErrCode AddPath(const Kza parent, const Kza daughter, const Path& path);


  ErrCode AddFissionYield(const Kza parent, const Kza daughter,
			  const int fissionType, const double yield);





  std::vector<std::pair<double,double> > GetDiscreteSpec(Kza parent, 
							 int specType);
  double GetDecayConstant(Kza parent);
  double GetDecayEnergy(Kza parent, int enType);
  double GetTotalDecayEnergy(Kza parent);

  std::vector<std::pair<double,double> > 
    GetFissionYield(Kza parent, int fissionType);
  double GetFissionYield(Kza parent, Kza daughter, int fissionType);

  std::vector<Kza> Daughters(Kza parent);

  /// LambdaEff calculates reaction rates.
  /** The reaction rate consists of the decay constant of the parent added to
   *  the radiation induced reaction rates:\n
   *    [INSERT EQUATION]
   *  The total reaction rate is returned.
   */
  ErrCode LambdaEff(Kza parent,const std::vector<double>& flux,double& result);

  ErrCode LambdaEff(Kza parent, Kza daughter, const std::vector<double>& flux, 
		    double& result);


  double LambdaEff(Kza parent, const std::vector<double>& flux,
		   std::vector<Kza>& daughters, std::vector<double>& dLambdas,
		   double& secLambda, FissionType ft=DefaultFT);

  double GetBratio(Kza parent, Kza daughter);

  std::vector<double> GetGroupStruct( GSType gst );
  void SetGroupStruct( GSType gst, std::vector<double>& gs);

  double GetSfbr(Kza parent);

 private:

  /// This data structure stores all of the nuclear data
  std::map<Kza,Parent> Data;

  /// A map to store the various energy group structures
  std::map<GSType, std::vector<double> > GroupStructs;
};

#endif
