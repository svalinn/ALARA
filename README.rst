$Id: README,v 1.5 2005-01-17 22:11:04 wilsonp Exp $

Analytic and Laplacian Adaptive Radioactivity Analysis (ALARA)
==============================================================

Thank you for your interest in ALARA.

The primary purpose of ALARA is to calculate the induced activation caused by neutron irradiation throughout a nuclear system (including fission reactors, fusion reactors, and accelerators). Some projects currently using ALARA include ARIES, IFMIF and the Z-Pinch Power Plant study.

Build and Install
-----------------
Requirements
____________
ALARA is written primarily in C++ with one or two FORTRAN77 routines. All development is done with the GNU C++ and FORTRAN compilers and it has been tested and is known to work on Linux and Solaris. Installing ALARA requires both a C++ and a FORTRAN compiler. 

Process
_______
1. Obtain the distribution from the appropriate source.
2. Unpack the distribution.
   %> gunzip -c alara-2.7.1.tar.gz | tar xf -
3. Go to newly created directory.
   %> cd alara-2.7.1
4. Configure ALARA for your system
   %> ./configure
   See below for options that you can give to configure.
5. Build the application
   %> make
6. Install the application
   %> make install

Configuration Options
____________________
The configure program provides a lot of options for customizing the compilation and installation of ALARA. To learn more about the full set of options, you should check the built-in help:

%> ./configure --help

Some common options are listed here:

--prefix=/your/path/here
    This option allows you to change the default location for the installation of ALARA and its accompanying files. The next section describes the different files that are installed and the directories in which they are installed. More control over the specific directories is also possible - see the built-in help for more information.

CXX=name_of_your_C++_compiler
F77=name_of_your_F77_compiler
    This option allows you to explicitly specify which C++ (or FORTRAN) compiler you want to use to compile ALARA. If left out, configure will search for default compilers.

Additional Notes on Installation
________________________________
A. When building ALARA, you should run 'configure' and 'make' from the
   main 'alara-2.x.y' directory and NOT from the 'src' directory.

B. There is currently no set of tests to be conducted with 'make
   check'.  Instead, some test/sample files are included in directory
   './sample'.  

   They can all be run in succession, with output stored in a
   directory "sample_out" by changing to the sample directory and
   executing the run_samples.sh script: 
	%> cd sample 
	%> ./run_samples.sh sample_out 
   A reference set of output is available in directory
   './sample/ref_out'.  You can expect difference between your test
   output and the reference output with respect to the timing results
   and the directory locations of output files.

   NOTE: the data in the sample folder is not suitable for real
   calculations and is a truncated library just for the purpose of
   these tests.

Building Documentation
----------------------

How to Use
----------

Contribute & Develop
--------------------

Additional Notes
----------------

In addition to building and installing the ALARA program, this package
includes an accessory program, dant2alara, and a pair of Perl scripts
for post-processing data.

dant2alara...
...is an interactive program for converting RTFLUX/ATFLUX files (from
DANTSYS and similar) to text based flux files for use in ALARA.  Since
ALARA can read RTFLUX files directly, this use may not be useful.  On
the other hand, the biological dose method requires ggamma source to
dose conversion factors from an ATFLUX type file, which can not be
handled directly by ALARA at this time.

extract_pathways...
.... is a Perl script that scans an ASCII tree file and finds all the
chains/pathways that result in a given isotope.

summary ....
... is a Perl script that extracts a summary of the output file, most
notably removing all results for individual isotopes and leaving all
totals.  You can also extract results for a single specific isotopes
by giving the argument "-iso" followed by the isotope in question
written as a lower case atomic symbol hyphenated with the mass
number. e.g. tritium is h-3.
