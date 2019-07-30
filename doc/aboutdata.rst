=======================================
Acquiring and Installing Data for ALARA
=======================================

Data Formats
============

	ALARA uses a native binary data format to accelerate data 
	lookup and retreival. Obviously, data is not distributed 
	in this format and needs to be converted, a process which 
	is done by ALARA istelf. However, ALARA only supports 
	specific native data formats for this conversion process. 
	(If you would like to have support for a different data 
	format, please contact the 
	`developers <mailto:wilsonp@engr.wisc.edu>`.) 

	In summary, ALARA supports the EAF_ (LIBOUT) and 
	IEAF_ (GENDF) formats for transmutation data, and 
	the ENDF_ format for decay and gamma source data. 

.. _EAF:

	**EAF Format** (Transmutation Data [also called LIBOUT format]) 

	The first data format supported bu ALARA is the so-called 
	EAF format. While EAF data itself can come in many 
	formats, this format is meant to describe the multi-group 
	cross-section format used by FISPACT. Here are the first 
	30 lines of the FENDL 2.0 Activation library with a 175 
	group vitamin-j structure in the EAF format.

::

	 *******************************************************************************
	 *  FENDLA-2.0(0)175Gp THE FUSION EVALUATED NUCLEAR DATA LIBRARY   APR 1996    *
	 *******************************************************************************
	   CULHAM COPY OF ACTIVATION AND TRANSMUTATION CROSS SECTION FILE FOR
	   FUSION REACTORS FOR USE BY EUROPEAN ACTIVATION SYSTEM (EASY)
	 -------------------------------------------------------------------------------
	  ORIGINAL DATA SOURCE: EAF-4.1 FILE, PRODUCED BY J.KOPECKY AND D.NIEROP
	  STARTER FOR THIS VERSION IS EAF-4.1 (JAN 96) MAIN AREAS OF IMPROVEMENT ARE:
	  1) UPDATE OF REACTIONS FOLLOWING THE LIST SELECTED AT THE DEL MAR AGM MEETING
	     DETAILED IN INDC(NDS)-352 WITH SOURCES DATA FROM:
	        EAF-4.1, ADL-3, FENDL/A-1.1, IRK, CRP, ADL-3/I, AND JENDL3.2/A
	  2) ADDITIONAL TARGET HO-164 AND HO*-164
	  3) 404 REACTIONS MERGED/REPLACED IN EAF-4.1 (STOP AT CM-248)
	  THESE GROUP CROSS SECTIONS HAVE BEEN CALCULATED AT CULHAM IN THE 175-GROUP
	  VITAMIN-J STRUCTURE AND WITH THE VITAMIN-E MICRO-FLUX WEIGHTING FUNCTION.
	 ###############################################################################
	   10010 1020  175   H   1  (N,G  )H   2                1.0000+00*
	  JEF-2.2

	 2.67571E-05 2.74338E-05 2.77099E-05 2.82188E-05 2.89280E-05 2.92357E-05
	 2.95285E-05 2.97939E-05 2.99368E-05 3.00611E-05 3.03956E-05 3.06170E-05
	 3.11367E-05 3.15726E-05 3.19902E-05 3.23954E-05 3.27796E-05 3.31301E-05
	 3.34419E-05 3.37059E-05 3.39940E-05 3.44710E-05 3.49471E-05 3.52980E-05
	 3.54198E-05 3.55118E-05 3.56592E-05 3.58352E-05 3.60194E-05 3.61905E-05
	 3.63575E-05 3.63948E-05 3.63581E-05 3.62999E-05 3.61767E-05 3.59677E-05
	 3.58238E-05 3.57321E-05 3.55879E-05 3.53810E-05 3.51832E-05 3.49968E-05
	 3.48454E-05 3.47706E-05 3.47426E-05 3.47031E-05 3.46198E-05 3.44854E-05
	 3.43325E-05 3.42378E-05 3.42540E-05 3.42722E-05 3.42896E-05 3.43066E-05
	 3.43285E-05 3.43475E-05 3.43785E-05 3.44599E-05 3.44539E-05 3.44157E-05
	 3.44285E-05 3.44484E-05 3.44916E-05 3.45820E-05 3.46764E-05 3.47571E-05

.. _IEAF:

	**IEAF Data** (Transmutation data)

	Support was added for the IEAF 2001 data in order to permit 
	modelling of applications with neutron energies up to 150 
	MeV. In particular, this supports the GENDF formatted 
	version of the IEAF data stored in file 3, section 5. The 
	following shows two sections of the GENDF file for 1H. 
	(Note that for processing, a single IEAF file should be 
	created as an ordered concatenation of all the 
	individual isotope files.

**lines 1-10**

::

	 1- H-  1 FZK,INPE                                                    1 0  0    0
	  1.001000+4 9.991700-1          0          1         -1         17 101 1451    1
	  3.000000+2  .000000+0        256          0        276          0 101 1451    2
	   .000000+0  .000000+0  .000000+0  .000000+0  .000000+0  .000000+0 101 1451    3
	   .000000+0  .000000+0  .000000+0  .000000+0  .000000+0  .000000+0 101 1451    4
	   .000000+0  .000000+0  .000000+0  .000000+0  .000000+0 1.00000+10 101 1451    5
	  1.000000-5 1.000000-1 4.140000-1 5.316000-1 6.826000-1 8.764000-1 101 1451    6
	  1.125000+0 1.445000+0 1.855000+0 2.382000+0 3.059000+0 3.928000+0 101 1451    7
	  5.043000+0 6.476000+0 8.315000+0 1.068000+1 1.371000+1 1.760000+1 101 1451    8
	  2.260000+1 2.902000+1 3.727000+1 4.785000+1 6.144000+1 7.889000+1 101 1451    9 

**lines 41-60**

::

	 4.800000+7 4.900000+7 5.000000+7 5.200000+7 5.400000+7 5.600000+7 101 1451   40
	 5.800000+7 6.000000+7 6.200000+7 6.400000+7 6.600000+7 6.800000+7 101 1451   41
	 7.000000+7 7.200000+7 7.400000+7 7.600000+7 7.800000+7 8.000000+7 101 1451   42
	 8.200000+7 8.400000+7 8.600000+7 8.800000+7 9.000000+7 9.200000+7 101 1451   43
	 9.400000+7 9.600000+7 9.800000+7 1.000000+8 1.020000+8 1.040000+8 101 1451   44
	 1.060000+8 1.080000+8 1.100000+8 1.120000+8 1.140000+8 1.160000+8 101 1451   45
	 1.180000+8 1.200000+8 1.220000+8 1.240000+8 1.260000+8 1.280000+8 101 1451   46
	 1.300000+8 1.320000+8 1.340000+8 1.360000+8 1.380000+8 1.400000+8 101 1451   47
	 1.420000+8 1.440000+8 1.460000+8 1.480000+8 1.500000+8  .000000+0 101 1451   48
	                                                                   101 0  0   49
	 1.001000+4  .000000+0          1          1          0        256 101 3  5   50
	 3.000000+2  .000000+0          2          1          2          1 101 3  5   51
	 9.999000-2 3.309216-1                                             101 3  5   52
	 3.000000+2  .000000+0          2          1          2          2 101 3  5   53
	 3.140000-1 1.101258-1                                             101 3  5   54
	 3.000000+2  .000000+0          2          1          2          3 101 3  5   55
	 1.176000-1 7.701200-2                                             101 3  5   56
	 3.000000+2  .000000+0          2          1          2          4 101 3  5   57
	 1.510000-1 6.797717-2                                             101 3  5   58
	 3.000000+2  .000000+0          2          1          2          5 101 3  5   59

.. _ENDF:

	**ENDF Format** (Decay and Gamma Data) 

	The only decay data format supported by ALARA is the raw 
	ENDF-6 format. in File 8, section (MT) 457. The following 
	shows two sections of the FENDL 2.0 Decay library in ENDF 
	format.

**lines 1-20**

::

	      1 FENDL/D-2.0
	  BASED ON EAF-4.1 DECAY DATA LIBRARY
	 0.10010E+040.99141E+00          0          0          0          17500 1451    1
	 0.00000E+000.00000E+00          0          0          0          07500 1451    2
	 0.0        0.0                  0          0          6          17500 1451    3
	 ------------------------------------------------------------------7500 1451    4
	  1-H -  1    DECAY DATA FOR GREAC LIBRARY  APRIL 1987.            7500 1451    5
	                    ## STABLE NUCLIDE ##                           7500 1451    6
	   FILE ONLY CONTAINS NEEDED DECAY DATA, OTHER VALUES SET TO ZERO. 7500 1451    7
	                    ## STABLE NUCLIDE ##                           7500 1451    8
	 ------------------------------------------------------------------7500 1451    9
	 0.0        0.0                  1        451         10          17500 1451   10
	                                                                   7500 1  0   11
	                                                                   7500 0  0   12
	                                                                      0 0  0   13
	 0.10020E+040.19828E+01          0          0          0          17501 1451    1
	 0.00000E+000.00000E+00          0          0          0          07501 1451    2
	 0.0        0.0                  0          0          6          17501 1451    3
	 ------------------------------------------------------------------7501 1451    4
	  1-H -  2    DECAY DATA FOR GREAC LIBRARY  APRIL 1987.            7501 1451    5

**lines 61-80**

::

	                                                                   131 1451   33
	   ENDF/B- V DATA PRODUCED BY COGEND (A.TOBIAS JAN-1984).          131 1451   34
	                                                                   131 1451   35
	 ***************************************************************** 131 1451   36
	                                                                   131 1451   37
	 THE DATA WERE COMPILED AT THE NEA DATA BANK ON 20-JUL-93          131 1451   38
	 USING THE CODE "CORDECAY" VERSION 1.2                             131 1451   39
	                                                                   131 1451   40
	 ***************************************************************** 131 1451   41
	                                                                   131 1451   42
	                                1        451         44          0 131 1451   43
	                                8        457          9          0 131 1451   44
	                                                                   131 1  0   45
	                                                                   131 0  0   46
	 1.003000+3 2.990140+0          0          0          0          1 131 8457   47
	 3.891050+8 6.311520+5          0          0          6          0 131 8457   48
	 5.706600+3 1.843710+0 0.000000+0 0.000000+0 0.000000+0 0.000000+0 131 8457   49
	 5.000000-1 1.000000+0          0          0          6          1 131 8457   50
	 1.000000+0 0.000000+0 1.857100+4 6.000000+0 1.000000+0 0.000000+0 131 8457   51
	 0.000000+0 1.000000+0          0          0          6          1 131 8457   52


Format Conversion
=================

	When looking up and retrieving data, ALARA uses its own 
	binary data format exclusively. This requires coversion 
	from the above native data formats to the ALARA v2 
	format. Although a problem can be setup whereby ALARA 
	is told to use data in a native format, the 
	implementation of this simply performs the conversion at 
	the beginning of the run and throws away the converted 
	library at the end. It is best to use ALARA separately 
	to quickly create an ALARA v2 binary library and then 
	install that library for future direct use by ALARA.

	A library can be converted by simply running ALARA with 
	an input file that contains only the 
	:doc:`convert_lib <usersguide/inputtext>` input token.

Cross-Section Installation
==========================

	Once nuclear data has been processed into its ALARA v2 
	binary form, it can be installed in the default location 
	for access by alara: prefix/lib/alara/$OSTYPE. (See the 
	:doc:`installation guide <installguide>` for more on 
	the directory structure.) If not placed in this location, 
	data is searched for either in the path defined by the 
	environment variable $ALARA_XSDIR or in the current 
	working directory. 

Specific Data Packages
======================

	The following links contain information for processing 
	supported files from specific data packages avaiable 
	through standard services, primarily 
	`RSICC <https://rsicc.ornl.gov/>`_ and the NEA Databank. 

		1. D00183 - FENDL 2.0 

