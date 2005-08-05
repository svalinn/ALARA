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

const int VITAMINJ_175 = 1;
const int IEAF_256 = 2;
const int CINDER_60 = 3;

// Library Types:
const LibFormat DECAY_ENDF_6 = 1;
const LibFormat EAF_4_1 = 2;
const LibFormat CINDER = 3;
const LibFormat ENDF_IEAF = 4;

// FEIND Error Codes:
const ErrCode FEC_NO_ERROR = 0;
const ErrCode FEC_UNKNOWN_FORMAT = 1;
const ErrCode FEC_FILE_OPEN = 2;
const ErrCode FEC_UNKNOWN_DECAY_MODE = 3;
const ErrCode FEC_BAD_GROUP_STRUCT = 4;

// Types of Decay Energies:
const int HEAVY_PARTICLES = 1;
const int LIGHT_PARTICLES = 2;
const int EM_RADIATION = 3;

// ***** DECAY MODES *****
const unsigned int GAMMA_DECAY         = 1;
 
/// Constant to represent beta decay.
const unsigned int BETA_DECAY          = 2;
 
/// Constant to represent electron capture.
const unsigned int ELECTRON_CAPTURE    = 3;
 
/// Constant to represent an isomeric transition
/** Basically the same as gamma decay.
 */
const unsigned int ISOMERIC_TRANSITION = 4;
 
/// Constant to represent alpha decay.
const unsigned int ALPHA_DECAY         = 5;

/// Constant to represent neutron emission.
/** Not the same as delayed neutron emission.
 */
const unsigned int NEUTRON_EMISSION    = 6;

/// Constant to represent spontaneous fission.
/** Spontaneous fission will be supported by the Fission set of classes.
 */
const unsigned int SPONTANEOUS_FISSION = 7;

/// Constant to represent proton emission.
const unsigned int PROTON_EMISSION     = 8;

/// Constant to represent beta decay, followed by a neutron emission.
/** This is delayed neutron emission.
 */
const unsigned int BETA_NEUTRON_EMIT   = 9;

/// Constant to represent beta decay, followed by an alpha emission.
const unsigned int BETA_ALPHA_EMIT     = 10;

/// Constant to represent positron decay, followed by an alpha emission.
const unsigned int POSITRON_ALPHA_EMIT = 11;

const unsigned int POSITRON_PROTON_EMIT = 12;
const unsigned int IT_ALPHA_EMIT = 13;
const unsigned int UNKNOWN = 0;

// *********************************

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
const unsigned int TOTAL_CS = 1;
const unsigned int NEUTRON_FISSION_CS = 2;
const unsigned int CHARGED_CS = 3;

#endif
