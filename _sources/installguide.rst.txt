================
Installing ALARA
================

Requirements
============

	ALARA is written primarily in C++ with one or two 
	FORTRAN77 routines. All development is done with the 
	GNU C++ and FORTRAN compilers and it has been tested 
	and is known to work on Linux and Solaris. 
	Installing ALARA requires both a C++ and a FORTRAN 
	compiler. 

Installation Process
====================

1.  Obtain the distribution from the appropriate `source <https://github.com/svalinn/ALARA.git>`_.

2.  Unpack the distribution::

	gunzip -c alara-2.7.1.tar.gz | tar xf -

3.  Go to newly created directory::

	cd alara-2.7.1

4.  Configure ALARA for your system (See below for options 
    that you can give to configure.)::

	./configure

5.  Build the application::

	make

6.  Install the application::

	make install


Installation Options
====================

The configure program provides a lot of options for customizing 
the compilation and installation of ALARA. To learn more about 
the full set of options, you should check the built-in help:
::

	./configure --help

Some common options are listed here: 

::

	--prefix=/your/path/here 

This option allows you to change the default location for the 
installation of ALARA and its accompanying files. The next 
section describes the different files that are installed and 
the directories in which they are installed. More control 
over the specific directories is also possible - see the 
built-in help for more information.

::

	CXX=name_of_your_C++_compiler
	F77=name_of_your_F77_compiler 

This option allows you to explicitly specify which C++ (or 
FORTRAN) compiler you want to use to compile ALARA. If left
out, configure will search for default compilers.

Intalled files and directories
==============================

 When installing ALARA, a number of files and directories are 
 installed in a number of different places. This section
 summarizes these directories which are all subdirectories of
 the "prefix" directory described above.

	**bindir** - prefix/bin 

	The following files are placed in this directory: 

	* `alara` - the main application 
	* `dant2alara` - a utility for converting DANTSYS 
	  rtflux/atflux files to a format suitable for ALARA 
	* `summary` - a Perl utility for extracting summaries 
	  of ALARA output files 
	* `extract_pathways` - a Perl utility for extracting 
	  pathways from ALARA tree files 

	**datadir** - prefix/share/alara/data 

	This directory contains the following machine-independent 
	files (generally ASCII text files). 

	* `ANS6_4_3.txt <https://raw.githubusercontent.com/svalinn/ALARA/master/data/ANS6_4_3.txt>`_ 
	  - Data for implementing contact dose based on standard ANS6.4.3. 
	* `IAEA.clearance.2004.Bq_kg <https://raw.githubusercontent.com/svalinn/ALARA/master/data/IAEA.clearance.2004.Bq_kg>`_
	  - Limits for determining the material clearance rating based on IAEA standards. 
	* `FetterC_hi.Ci_m3.wdr <https://raw.githubusercontent.com/svalinn/ALARA/master/data/FetterC_hi.Ci_m3.wdr>`_ 
	  (and `FetterC_lo.Ci_m3.wdr <https://raw.githubusercontent.com/svalinn/ALARA/master/data/FetterC_lo.Ci_m3.wdr>`_ ) - 
	  Limits for determining the waste disposal rating based 
	  on the high (and low) values suggested by Fetter for 
	  each isotope. 
	* `NRCA.Ci_m3.wdr <https://raw.githubusercontent.com/svalinn/ALARA/master/data/NRCA.Ci_m3.wdr>`_ 
	  (and `NRCC.Ci_m3.wdr <https://raw.githubusercontent.com/svalinn/ALARA/master/data/NRCC.Ci_m3.wdr>`_ )
	  - Limits for determining the waste disposal rating based on the 
	  class A (and class C) definitions provided by the U.S. Nuclear 
	  Regulator Comissions. 
	* `elelib.std <https://raw.githubusercontent.com/svalinn/ALARA/master/data/elelib.std>`_ 
	  - A file in ALARA's element library format 
	  describing the standard natural isotopic abundances 
	  of all elements. 
	* `matlib.sample <https://raw.githubusercontent.com/svalinn/ALARA/master/data/matlib.sample>`_ 
	  - A file in ALARA's material library 
	  format to show samples of how materials can be 
	  defined and including some commong fusion materials. 

	**xsdir** - prefix/lib/alara/$OSTYPE

	This directory is created to store platform-dependent data, 
	namely binary ALARA v.2 format library files. The name of 
	this directory is based on the environment variable $OSTYPE 
	and the full path is compiled into the alara application 
	as a default location for finding cross-section data. No 
	files are put into this directory since acquiring and 
	installing data is a separate activity. (See the 
	:doc:`data guide <aboutdata>` for more on nuclear data.) 

