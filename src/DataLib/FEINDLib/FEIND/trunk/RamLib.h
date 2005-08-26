#ifndef __RAMLIB_H
#define __RAMLIB_H

#include <map>
#include <vector>
#include <utility>

#include "FeindNs.h"
#include "Parent.h"
#include "exception/ExInclude.h"

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
  /** This function adds a parent cross-section to the library. The new cross
   *  section can either replace or be added to the existing cross-section
   *  using the add argument.
   *
   *  \param[in] parent
   *  Kza of the parent isotope
   *
   *  \param[in] csType
   *  The type of parent cross-section to add
   *
   *  \param[in] cs
   *  XSec object that will be added to the RamLib
   *
   *  \param[in] add
   *  Flag to indicate whether the cross section should replace the existing
   *  one or be added to it. If this flag is set to true, and no cross-section
   *  currently exists, the function simply uses cs.
   */
  void SetPCs(const Kza parent, XSecType csType, XSec cs, bool add=false) 
    throw(ExXsecSize, ExEmptyXSec);

  XSec GetPCs(Kza parent, int csType);

  void SetDCs(const Kza parent, const Kza daughter, XSecType csType,
		 XSec cs, bool add=false) throw(ExXsecSize, ExEmptyXSec);

  XSec GetDCs(Kza parent, Kza daughter, int csType);
  
  void AddPath(const Kza parent, const Kza daughter, const Path& path);



  //**************************//
  //*** GET/SET DECAY DATA ***//
  //**************************//

  void AddDecayConstant(Kza parent, const double constant);
  double GetDecayConstant(Kza parent);

  void AddDecayEnergy(const Kza parent, const int enType, 
			 const double energy);
  double GetDecayEnergy(Kza parent, int enType);  

  double GetTotalDecayEnergy(Kza parent);

  /// Add a spectrum.
  void AddSpectrum(const Kza parent, const int specType, 
		      const Spectrum& spec);

  const std::vector<std::pair<double,double> >& GetDiscreteSpec(Kza parent, 
								int specType);

  /// Add a decay mode to the library
  /** This function will automatically add the appropriate information for
   *  secondary radiation as well
   */
  void AddDecayMode(Kza parent, DecayModeType decayMode, int dIso, double br)
    throw(ExDecayMode);
  
  double GetBratio(Kza parent, Kza daughter);

  /// Get the total spontaneous fission branching ratio for a parent
  double GetSfbr(Kza parent);



  //****************************//
  //*** SET/GET FISSION DATA ***//
  //****************************//

  void AddFissionYield(const Kza parent, const Kza daughter,
			  const int fissionType, const double yield);
  double GetFissionYield(Kza parent, Kza daughter, int fissionType);



  //****************************************************//
  //*** CALCULATING PRODUCTION AND DESTRUCTION RATES ***//
  //****************************************************//

  void LambdaEff(Kza parent,const std::vector<double>& flux,double& result);

  void LambdaEff(Kza parent, Kza daughter, const std::vector<double>& flux, 
		    double& result);

  /// Function to calculate total and secondary reaction rates
  /** This reaction rate function returns the total destruction rate associated
   *  with the given parent isotope and the flux. It also returns reactions
   *  rates corresponding to the rate at which each daughter is produced. 
   *  Finally a secondary reaction rate is also provided.
   *  
   *  \param[in]  parent 
   *  Kza of the parent isotope
   *
   *  \param[in]  flux 
   *  Group-wise neutron flux. This flux should match the group structure of
   *  the data in the RamLib, otherwise this function will throw exceptions.
   *
   *  \param[out] daughters 
   *  Vector containing Kzas of the daughters of the parent.
   *
   *  \param[out] dLambdas 
   *  Reaction rate coefficients corresponding to each daughter in daughters.
   *  These values are calculated from the following formula:
   *  \f[ \lambda_i = \sum_{j=1}^{n_g} (\sigma_{i,j} \phi_j) + \lambda_d b_i
   *  \f]
   *  where:\n
   *  \f$ \lambda_i \f$ is the reaction rate coefficient of a specific 
   *  daughter\n
   *  \f$ n_g \f$ is the number of energy groups in the flux \n
   *  \f$ \sigma_i \f$ is the total parent daughter cross-section \n
   *  \f$ \phi \f$ is the flux\n
   *  \f$ \lambda_d \f$ is the parent decay constant\n
   *  \f$ b_i \f$ is the branching ratio associated with this parent and 
   *  daughter
   *
   *  \param[out] secLambda 
   *  The sum of all of the reaction rate coefficients which correspond to the
   *  production of secondary reaction products. This function is calculated
   *  using the data in the RamLib along with the total reaction rate 
   *  coefficient (which is also returned by this function). The following
   *  formula is used to calculate this argument: 
   *  \f[ \lambda_{sec} = \sum_{i = 1}^{n_d}
   *      \left( \sum_{j = 1}^{n_g} (\sigma_{i,j} \phi_j) + \lambda_d b_i 
   *      \right) - \lambda_{total} \f]
   *  where:\n
   *  \f$ \lambda_{sec} \f$ is the secondary reaction rate coefficient\n
   *  \f$ n_d \f$ is the number of daughters of the parent\n
   *  \f$ n_g \f$ is the number of energy groups in the flux\n
   *  \f$ \sigma_i \f$ is the total parent daughter cross-section\n
   *  \f$ \phi \f$ is the flux\n
   *  \f$ \lambda_d \f$ is the parent decay constant\n
   *  \f$ b_i \f$ is the branching ratio associated with this parent\n
   *  \f$ \lambda_{total} \f$ is the total destruction rate of the parent
   *
   *  \param[in] ft 
   *  The fission type to use when calculating the reaction rates. This 
   *  information is used to identify a specific fission yield to use during
   *  these calculations. This fission yield is multipled by the total fission
   *  cross-section to characterize the liklihood that each possible fission
   *  daughter is produced.
   *  
   *  \return This function returns the total destruction rate associated with
   *  this parent and flux. The reaction rate is calculated using the
   *  following equation:\n
   *  \f[ \lambda_{total} = \sum_{j = 1}^{n_g} (\sigma_j \phi_j) + \lambda_d 
   *  \f]
   *  where:\n
   *  \f$ \lambda_{total} \f$ is the total destruction rate of the parent\n
   *  \f$ n_g \f$ is the number of energy groups in the flux\n
   *  \f$ \sigma \f$ is the total parent cross-section\n
   *  \f$ \phi \f$ is the flux
   *  \f$ \lambda_d \f$ is the parent decay constant
   */
  double LambdaEff(Kza parent, const std::vector<double>& flux,
		   std::vector<Kza>& daughters, std::vector<double>& dLambdas,
		   double& secLambda, FissionType ft=DefaultFT) 
    throw(ExXsecSize, ExEmptyXSec);

  /// Function to calculate production rates for adjoint calculations.
  /** This function calculates the total destruction rate associated with a 
   *  given daughter, just like all of the lambda effective functions. However,
   *  in addition, it also returns production rates for the provided daughter
   *  isotope. The production rates correspond to each possible parent of this
   *  daughter, and characterize the rate at which the given daughter is
   *  produced by each parent.
   *
   *  \param[in] daughter
   *  The daughter isotope
   *
   *  \param[in] flux 
   *  Group-wise neutron flux. This flux should match the group structure of
   *  the data in the RamLib, otherwise this function will throw exceptions.
   *
   *  \param[out] parents
   *  A vector containing all of the possible parents of the given daughter.
   *
   *  \param[out] pLambdas
   *  A vector storing the production rates associate with each parent in the
   *  parents vector.
   *  \f[ \lambda_i = \sum_{j=1}^{n_g} (\sigma_{i,j} \phi_j) + \lambda_d b_i
   *  \f]
   *  where:\n
   *  \f$ \lambda_i \f$ is the production rate of the daughter from parent
   *  \f$ i \f$
   *  \f$ n_g \f$ is the number of energy groups in the flux \n
   *  \f$ \sigma_i \f$ is the total parent daughter cross-section \n
   *  \f$ \phi \f$ is the flux\n
   *  \f$ \lambda_d \f$ is the parent decay constant\n
   *  \f$ b_i \f$ is the branching ratio associated with this parent and 
   *  daughter
   *
   *  \param[in] ft 
   *  The fission type to use when calculating the reaction rates. This 
   *  information is used to identify a specific fission yield to use during
   *  these calculations. This fission yield is multipled by the total fission
   *  cross-section to characterize the liklihood that each possible fission
   *  daughter is produced.
   *
   *  \return This function returns the total destruction rate associated with
   *  this daughter and flux. The reaction rate is calculated using the
   *  following equation:\n
   *  \f[ \lambda_{total} = \sum_{j = 1}^{n_g} (\sigma_j \phi_j) + \lambda_d 
   *  \f]
   *  where:\n
   *  \f$ \lambda_{total} \f$ is the total destruction rate of the daughter\n
   *  \f$ n_g \f$ is the number of energy groups in the flux\n
   *  \f$ \sigma \f$ is the total daughter cross-section\n
   *  \f$ \phi \f$ is the flux
   *  \f$ \lambda_d \f$ is the daughter's decay constant
   */
  double ProdRates(Kza daughter, const std::vector<double>& flux,
		   std::vector<Kza>& parents, std::vector<double>& pLambdas,
		   FissionType ft=DefaultFT) throw(ExEmptyXSec, ExXsecSize);

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
