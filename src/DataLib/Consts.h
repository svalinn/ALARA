#ifndef CONSTS_H
#define CONSTS_H

const unsigned int NUM_ELEMENTS = 110;

// Energy Group Structures:

/// The Group Struct Types
/** This enum just contains codes for each different groups structure used
 *  in FEINDs various parsers
 */
enum GSType{ 
  VITAMIN_J,      /**< 175 group Vitamin - J structure, used by EAF (fendl) */
  IEAF,           /**< 275 group structure for IEAF data */
  CINDER_NEUTRON, /**< 63 group Cinder neutron energy group structure */
  CINDER_GAMMA    /**< 25 group Cinder gamma energy group structure */
};

/// FEIND format identifiers
/** A single identifier exists to correspond to each parser.
 */
enum FEINDFormat
  {
    DECAY_ENDF_6 = 0, /**< ENDF VI decay format */
    EAF_4_1 = 1,      /**< EAF 4.1 format for cross-sections */
    CINDER = 2,       /**< The CINDER data format, which contains decay data,
		       *   cross sections and fission yields. */
    ENDF_IEAF = 3     /**< The ENDF VI format for cross-sections */
  };

/// FEIND error identifiers
/** These are error codes for each type of exception that can be thrown when
 *  FEIND runs. These error codes are used as return values when programs run,
 *  and will eventually be used to proived "exception handling" in fortran.
 */
enum FEINDErrorType
  {
    /// Error code for an unknown, or not built in error type.
    FEIND_UNKNOWN_ERROR,

    /// Error code for unknown formats
    /** Usually encountered if a user specifies a format type that FEIND does
     *  not understand. Corresponds to the ExFormat exception type.
     */
    FEIND_FORMAT_ERROR,

    /// Error code for unknown decay modes
    /** Usually encoutered when a parser is loading decay data, and asks
     *  FEIND to add a certain decay mode to the RamLib. Corresponds to the
     *  ExDecayMode exception type.
     */
    FEIND_DECAYMODE_ERROR,

    /// Error code for problems opening files
    /** Usually encountered when the parser attempts to open a file that does
     *  not exist. Corresponds to the ExFileOpen exception type.
     */
    FEIND_FILEOPEN_ERROR,
    
    /// Error code for invalid options sent to parsers.
    /** This error type is used when programs send invalid arguments to the
     *  parsers. Corresponds with the ExInvalidOption exception type.
     */
    FEIND_INVALIDOPTION_ERROR,

    /// Error code for operations performed on XSec's of different sizes.
    /** Corresponds to the ExXsecSize exception type.
     */
    FEIND_XSECSIZE_ERROR,

    /// Error code for operations performed on empty cross-sections.
    /** Users must initialize the XSec object to something before it can be
     *  used. Corresponds to the ExEmptyXSec exception type.
     */
    FEIND_EMPTYXSEC_ERROR
  };

/// Types of decay energies
enum DecayEnergyType
  {
    HEAVY_PARTICLES = 0, /**< Heavy particles include alphas, protons... */
    LIGHT_PARTICLES = 1, /**< Electrions, positrons... */
    EM_RADIATION = 3     /**< Gammas, x-rays... */
  };

/// Types of decay modes
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

    /// Constant to represent positron decay, followed by proton emission.
    POSITRON_PROTON_EMIT = 12,

    /// Constant to represent simultaneous isomeric transitions and alpha decay
    IT_ALPHA_EMIT        = 13
  };

/** \name Common particle KZA's
 *  These constants simply represent Kzas for commonly used particles. They
 *  can be used to make code more readable.
 */
//@{
const unsigned int PROTON = 10010;
const unsigned int NEUTRON = 10;
const unsigned int DEUTERON = 10020;
const unsigned int TRITON = 10030;
const unsigned int HELIUM3 = 20030;

// Helium 4 and Alpha have the same value!
//   So they may be used interchangably...
const unsigned int HELIUM4 = 20040;
const unsigned int ALPHA = 20040;
//@}

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

/// Cross-section identifiers
enum XSecType
  {
    TOTAL_CS = 1,
    NEUTRON_FISSION_CS = 2,
    CHARGED_CS = 3
  };

#endif
