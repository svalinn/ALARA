#ifndef __PARENT_H
#define __PARENT_H

#include "FeindNs.h"
#include "XSec.h"

#include <vector>
#include <map>
#include <utility>

/** \file Parent.h
 *  \brief This file stores classes which store all of the relevent nuclear
 *         data for a specific isotope.
 */

/// Stores path information about a given reaction.
/** The path simply governs which projectile causes a reaction, and which 
 *  particles are emitted.
 */
class FEIND::Path
{
 public:
  /// The type of projectile that causes this reaction.
  Kza Projectile;
  
  /// Emitted stores how many particles of each type are emitted.
  /** For example, Emitted[10010] = 2\n
   *  would mean that 2 protons were emitted.
   */
  std::map<Kza, int> Emitted;

  /// The groupwise cross-section for this reaction.
  XSec CrossSection;
};

/// Stores data about a continuous decay spectrum
class FEIND::ContSpec
{
 public:
  /// The a vector of energy/fraction pairs for this spectrum.
  std::vector< std::pair<double, double> > Point;

  /// The method of interpolation used to connect the various points.
  /** For example, linear, logarithmic, spline (Thanks NEEP 271)
   */
  int IntMethod;
  
  void Clear();
};

/// Stores information about decay spectra.
/** These decay spectra may be discrete continuous or groupwise.
 */
class FEIND::Spectrum
{
 public:
  /// A vector containing the various continuous spectra.
  /** Each member of this vector will contain a different region of 
   *  interpolation. Each region contains a vector of points along with an
   *  interpolation method. This allows you to use different methods for a
   *  single spectrum.
   */
  std::vector< ContSpec > Continuous;
  
  /// A vector of energy/fraction pairs for discrete spectrum entries.
  /** This vector is simply a list of points for each energy and intensity.
   */
  std::vector< std::pair<double, double> > Discrete;

  /// This vector stores a group wise spectrum.
  /** To save memory the energy group structure is not build into this class.
   *  The user must be aware of this information himself.
   */
  std::vector<double> GroupWise;

};

/// Stores all nuclear data regarding a specific daughter of an isotope.
class FEIND::Daughter
{
 public:
  Daughter();

  /// This map contains special cross sections for this parent/daughter pair.
  /** For example, if you wanted the fission, fusion, total cross section...
   */
  std::map<int, XSec > CrossSections;

  std::vector<Path> PathList;

  /// The fraction of decays that result in the production of this daughter.
  /** Set this to 0 for no decay.
   */
  double BranchingRatio;

  /// The decay mode that gets from parent to daughter.
  /** This member is set to zero when this daughter is not produced as a
   *  result of decay.
   */
  //double DecayMode;

  std::map<int, double> FissionYield;
};

/// Stores all nuclear data for a single isotope
/** The RamLib contains a map of parent objects. Information is added to these
 *  through the RamLibs interface.
 */
class FEIND::Parent
{
 public:
  Parent();

  /// This map contains special cross sections for this parent.
  /** For example, if you wanted the fission, fusion, total cross section...
   */
  std::map<int, XSec > CrossSections;
  
  /// The list of daughters.
  std::map<Kza,Daughter> Daughters;

  /// DecayConstant stores the decay constant for this parent.
  /** This member should be set to 0 for parents that are stable and do not
   *  Decay.
   */
  double DecayConstant;

  /// DecayEnergies stores the various decay energies.
  /** The map key is the type of particle that is emitted (photons, betas...)
   */
  std::map<int, double> DecayEnergies;

  /// The decay spectrum for this parent.
  /** This map is organized in emitted particle type/Spectrum pairs.
   */
  std::map<int, Spectrum> SpecList;

  /// A list of possible decay modes for an isotope
  std::vector<DecayModeType> DecayModes;

  /// Spontaneous fission branching ratio:
  /** This number should be set to zero for no spontaneous fission.
   */
  double Sfbr;

  
};

#endif

