$Id: NEWS,v 1.4 2004/01/29 08:09:13 wilsonp Exp $

Version 2.7.1
	
BUGS FIXED
        * Removed debugging output at response reminders
        * Fixed ADJOINT libraries to work with v.2 binary libraries [PW] (10/20/2003)

NEW FEATURES
	* Machine-dependent data files (binary ALARA libraries) are searched for in a path
	  defined by ALARA_XSDIR environment variable and
	  ${exec_prefix}/lib/alara/$OSTYPE [PW] (10/27/2003)

CODE UPDATES
	
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 	

Version 2.7.0
	
BUGS FIXED
        * Fixed ADJOINT libraries to work with v.2 binary libraries [PW] (10/20/2003)
        * Reversal of multiplication made on 6/03/03 undone; group order
          in GammaSrc and adjoint input file is the same [AV] (6/18/2003)
        * Fixed indexing of average energy vector for decay types [PW] (06/10/2003)
        * Multiplication of importance and gamma source for bio dose
          calculation is reversed to account for order of gp struc [AV] (6/03/03) 
        * Bio dose results are now divided by detector volume (hence
          new input parameter needed for bio dose request line) [AV] (6/03/03) 
        * Modified bio dose calculation to use volume instead 
          of uservol [AV] (3/25/03) 	
        * Change parameter passed to Result::write so that uservol is 
	  correctly processed [AV] (09/18/2002)	
	* Util/input.C:tokentype() - token conversion to lower case only
	  changed first character [AV] (08/13/2002)
        * Command-line argument handling: when extra argument expected (-v, -t)
	  for last option but not given, casuses SEGFAULT (core dump).  Check
	  for last command-line option and error. [AV] (09/09/2002)
	* Fixed units for contact dose [PW] (01/13/2003)
	

NEW FEATURES
	* Machine-dependent data files (binary ALARA libraries) are searched for in a path
	  defined by ALARA_XSDIR environment variable and
	  ${exec_prefix}/lib/alara/$OSTYPE [PW] (10/27/2003)
	* Response type reminders now appear above each and every table [PW] (6/12/2003)
	* Machine-independent data files (WDR, ANS 6.4.3, etc) are searched for in a path
	  defined by ALARA_DATADIR environment variable and ${prefix}/share/alara/data
	  defined by configure [PW] (6/12/2003)
	* Implemented Biological/Adjoint/Folded dose in Volumes [AV] (3/??/03)
	* Direct reading of RTFLUX by ALARA [MF] (08/23/02)
	* Read user-defined total volume in 'loading' input block. [AV] (09/09/2002)
	* Use user-defined total volume with special 'units' entry of
	  output block to give volume integrated results. [AV] (09/09/2002)
        * Adjoint dose calculation for volume resolution [AV] (12/02/2002)

CODE UPDATES
	* Update for addition of software for producing collaboration
	  graphs in doxygen called 'dot'  [PW] (08/11/2002)
	* Changed code to use GNU autoconf/make, etc... [EK,PW] (01/06/2003)
	
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 	

Version 2.6.0
	
* Start of ChangeLog (08/08/2002)

BUGS FIXED


NEW FEATURES
	* Squeeze multiple zeros out of ALARALib [PW] (08/09/2002) 
	  THIS WILL REQUIRE CHANGE IN MINOR REVISION NUMBER


CODE UPDATES
	* All class comments converted to Doxygen compliance
	* Added target to Makefile to make documentation with Doxygen
	* Improved C++ ISO standards compliance
	  - use correct headers
	  - completely specify entities from std namespace
	  - import various stream entities from std namespace
	* Added ChangeLog and TODO files
	
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 	

Version 2.5.3

BUGS FIXED
	* ASCII IEAF library was not being read properly - missing extra
	  line at end of each isotope's data

NEW FEATURES
	* NONE

CODE UPDATES
	* NONE
	
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 

Version 2.5.2

BUGS FIXED
	* Atomic X-ray data was not being included in gamma library when
	  translating from IEAF libarary

NEW FEATURES
	* NONE

CODE UPDATES
	* Added target to compile condor binary (at CAE)

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 	

Version 2.5.1

BUGS FIXED
	* Atomic X-ray data was not being included in gamma library
	* Incorrect results for schedules with varying fluxes.  Previous
	  bug fix (v2.?.?) was incomplete.  Diagonal matrix element was
	  not based on correct flux.
	* Simplified/corrected interpolation/extrapolation for contact
	  dose calculation.

NEW FEATURES
	* Added mode to only build chains and solve without output.  This
	  is designed to facilitate Condor operation that might have
	  problems closing the binary file and immediately reopening it.
	
CODE UPDATES
	* Began introducing namespace specifiers for included elements of
	  std namespace.

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 

Version 2.5.0

* Minor version changed due to new features

BUGS FIXED
	* Core dump when invalid included input file. Check for NULL
	  istream pointer.


NEW FEATURES
	* Added contact dose output.
	* Added binary tree file output


CODE UPDATES
	* Added target to use makedepend for automatic dependency
	  generation

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 

Version 2.4.6 Patches

BUGS FIXED
	* Total cross-section of IEAF library is incorrectly calculated by
	  summing partial cross-sections.  This over counts the total
	  transmutation cross-section.  Now correctly read and locate
	  total cross section from library.

	* Various memory leaks found and plugged.
	
NEW FEATURES

CODE UPDATES

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 


