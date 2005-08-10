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

  /// Return all of the possible parents of a given daughter
  /** This function requires the Adjoint map to be constructed in order to
   *  work. The adjoint map can be built using the ConstructAdjoint()
   *  member function.
   */
  const std::vector<Kza>& Parents(Kza daughter);

  /// Return a list of the daughters of a given parent
  std::vector<Kza> Daughters(Kza parent);



  //******************************//
  //*** GET/SET CROSS SECTIONS ***//
  //******************************//

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
  ErrCode SetPCs(const Kza parent, const int csType, XSec cs, bool add=false);

  XSec GetPCs(Kza parent, int csType);

  ErrCode SetDCs(const Kza parent, const Kza daughter, const int csType,
		 XSec cs, bool add=false);

  XSec GetDCs(Kza parent, Kza daughter, int csType);
  
  ErrCode AddPath(const Kza parent, const Kza daughter, const Path& path);



  //**************************//
  //*** GET/SET DECAY DATA ***//
  //**************************//

  ErrCode AddDecayConstant(Kza parent, const double constant);
  double GetDecayConstant(Kza parent);

  ErrCode AddDecayEnergy(const Kza parent, const int enType, 
			 const double energy);
  double GetDecayEnergy(Kza parent, int enType);  

  double GetTotalDecayEnergy(Kza parent);

  /// Add a spectrum.
  ErrCode AddSpectrum(const Kza parent, const int specType, 
		      const Spectrum& spec);

  std::vector<std::pair<double,double> > GetDiscreteSpec(Kza parent, 
							 int specType);

  /// Add a decay mode to the library
  /** This function will automatically add the appropriate information for
   *  secondary radiation as well
   */
  ErrCode AddDecayMode(Kza parent, int decayMode, int dIso, double br);

  double GetBratio(Kza parent, Kza daughter);

  /// Get the total spontaneous fission branching ratio for a parent
  double GetSfbr(Kza parent);



  //****************************//
  //*** SET/GET FISSION DATA ***//
  //****************************//

  ErrCode AddFissionYield(const Kza parent, const Kza daughter,
			  const int fissionType, const double yield);
  double GetFissionYield(Kza parent, Kza daughter, int fissionType);
  std::vector<std::pair<double,double> > 
    GetFissionYield(Kza parent, int fissionType);



  //****************************************************//
  //*** CALCULATING PRODUCTION AND DESTRUCTION RATES ***//
  //****************************************************//

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


  double ProdRates(Kza daughter, const std::vector<double>& flux,
		   std::vector<Kza>& parents, std::vector<double>& pLambdas,
		   FissionType ft=DefaultFT);



  //********************************//
  //*** GET/SET GROUP STRUCTURES ***//
  //********************************//

  std::vector<double> GetGroupStruct( GSType gst );

  void SetGroupStruct( GSType gst, std::vector<double>& gs);



  //*********************//
  //*** MISCILLANEOUS ***//
  //*********************//

  /// Construct a map of daughters to their parents
  /** This map will be useful in calculating production rates for isotopes.
   */
  void ConstructAdjoint();

 private:

  /// This data structure stores all of the nuclear data
  std::map<Kza,Parent> Data;

  /// Map of daughters to their possible parents
  /** This map is useful for performing calculations involving production
   *  rates. The map is formed ahead of time so that it will be possible to
   *  quickly access the parents of a given isotope. \n\n
   *
   *  This map is set up by the ConstructAdjoint() member.
   */
  std::map<Kza, std::vector<Kza> > Adjoint;

  /// A map to store the various energy group structures
  std::map<GSType, std::vector<double> > GroupStructs;
};

#endif
