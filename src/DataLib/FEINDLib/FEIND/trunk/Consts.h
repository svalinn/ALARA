#ifndef __CONSTS_H
#define __CONSTS_H

const unsigned int NUM_ELEMENTS = 110;

// Energy Group Structures:

/// The Group Struct Types
/** This enum just contains codes for each different groups structure used
 *  in FEINDs various parsers
 */
enum GSType{ 
  VITAMIN_J,      // 175 group Vitamin - J structure, used by EAF (fendl)
  IEAF,          
  CINDER_NEUTRON, // 63 group Cinder neutron energy group structure
  CINDER_GAMMA    // 25 group Cinder gamma energy group structure
};

// Library Types:
enum FEINDFormat
  {
    DECAY_ENDF_6 = 0,
    EAF_4_1 = 1,
    CINDER = 2,
    ENDF_IEAF = 3
  };

enum FEINDErrorType
  {
    FEIND_UNKNOWN_ERROR,
    FEIND_FORMAT_ERROR,
    FEIND_DECAYMODE_ERROR,
    FEIND_FILEOPEN_ERROR,
    FEIND_INVALIDOPTION_ERROR,
    FEIND_XSECSIZE_ERROR,
    FEIND_EMPTYXSEC_ERROR
  };

// Types of Decay Energies:
enum DecayEnergyType
  {
    HEAVY_PARTICLES = 0,
    LIGHT_PARTICLES = 1,
    EM_RADIATION = 3
  };

enum DecayModeType
  {

    // ***** DECAY MODES *****
    UNKNOWN              = 0,

    GAMMA_DECAY          = 1,
 
    /// Constant to represent beta decay.
    BETA_DECAY           = 2,
 
    /// Constant to represent electron capture.
    ELECTRON_CAPTURE     = 3,
    
    /// Constant to represent an isomeric transition
    /** Basically the same as gamma decay.
     */
    ISOMERIC_TRANSITION  = 4,
    
    /// Constant to represent alpha decay.
    ALPHA_DECAY          = 5,
    
    /// Constant to represent neutron emission.
    /** Not the same as delayed neutron emission.
     */
    NEUTRON_EMISSION     = 6,
    
    /// Constant to represent spontaneous fission.
    /** Spontaneous fission will be supported by the Fission set of classes.
     */
    SPONTANEOUS_FISSION  = 7,
    
    /// Constant to represent proton emission.
    PROTON_EMISSION      = 8,
    
    /// Constant to represent beta decay, followed by a neutron emission.
    /** This is delayed neutron emission.
     */
    BETA_NEUTRON_EMIT    = 9,
    
    /// Constant to represent beta decay, followed by an alpha emission.
    BETA_ALPHA_EMIT      = 10,
    
    /// Constant to represent positron decay, followed by an alpha emission.
    POSITRON_ALPHA_EMIT  = 11,
    
    POSITRON_PROTON_EMIT = 12,

    IT_ALPHA_EMIT        = 13
  };

// ***** Types of Particles *****
const unsigned int PROTON = 10010;
const unsigned int NEUTRON = 10;
const unsigned int DEUTERON = 10020;
const unsigned int TRITON = 10030;
const unsigned int HELIUM3 = 20030;

// Helium 4 and Alpha have the same value!
//   So they may be used interchangably...
const unsigned int HELIUM4 = 20040;
const unsigned int ALPHA = 20040;

// The Following particles are considered special...ie no Kza:
const unsigned int GAMMA = 1;
const unsigned int BETA = 2;
const unsigned int ELECTRON = 3;
const unsigned int XRAY = 4;
const unsigned int SF_FRAGMENTS = 5;
const unsigned int POSITRON = 6;
const unsigned int FISSION_DAUGHTER = 7;

// ***************************

/// Types of Fission Yields:
enum FissionType
  {
    NO_FISSION,
    FISSION_FAST,
    FISSION_THERMAL,
    FISSION_HOT,
    FISSION_SF
  };

// Special Cross Sections:
enum XSecType
  {
    TOTAL_CS = 1,
    NEUTRON_FISSION_CS = 2,
    CHARGED_CS = 3
  };

#endif
