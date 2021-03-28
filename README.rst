$Id: README,v 1.5 2005-01-17 22:11:04 wilsonp Exp $

Analytic and Laplacian Adaptive Radioactivity Analysis (ALARA)
==============================================================

Thank you for your interest in ALARA.

The primary purpose of ALARA is to calculate the induced activation caused by neutron irradiation throughout a nuclear system (including fission reactors, fusion reactors, and accelerators). Some projects currently using ALARA include ARIES, IFMIF and the Z-Pinch Power Plant study.

Special Features of ALARA
~~~~~~~~~~~~~~~~~~~~~~~~~

ALARA takes full advantage of being a newly developed code by implementing the best features of the previous generation of activation codes and, at the same time, addressing their various weaknesses. Furthermore, an entirely new set of features has been implemented and continued development will allow it to respond to the evolving needs of the entire user community. ALARA's three main design principles are accuracy, speed, and usability. ALARA matches the previous generation of activation codes in accuracy and is much faster. This is achieved by adaptively choosing the mathematical method at the smallest resolution in order to optimize both speed and accuracy.

ALARA distinguishes itself by including unique capabilities that are useful to engineers and designers:

* unlimited number of reaction channels
* exact modeling of hierarchical arbitrary irradiation schedules
* reverse calculation mode

ALARA implements many of the standard features of activation codes including:

* multi-point (3-D) solutions in a variety of geometries
* accurate solution of loops in activation trees
* exact modeling of multi-level pulsing irradiation histories
* user-defined calculation precision/accuracy
* tracking the accumulation of light ions
* straightforward, user-friendly input file creation
* full, easy-to-read activation tree output (not just pathway analysis)
* flexible output options NOW including the direct calculation of waste disposal ratings and clearance indices.

Build and Install
~~~~~~~~~~~~~~~~~

Requirements
------------

ALARA is written primarily in C++ with one or two FORTRAN77 routines.
All development is done with the GNU C++ and FORTRAN compilers and it has been tested and is known to work on Linux and Solaris.

The following pieces of software are required in order to build ALARA:

* C++ compiler
* Fortran compiler
* CMake

Installation Process
--------------------

Here are some commands that can be used to build ALARA.
::

    $ INSTALL_PREFIX=${PWD}
    $ git clone https://github.com/svalinn/ALARA
    $ mkdir bld
    $ cd bld
    $ cmake ../ALARA -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
    $ make
    $ make install

Test Install
------------

Several sample ALARA problems are installed to the ``sample`` directory as part of the install process.
Here are some commands that can be used to run these sample problems.
::

    $ cd ../sample
    $ bash run_samples.sh

If the sample problems run without errors, the following output should appear on the command line.
::

    ALARA 2.9.2
    sample1
    sample2
    sample3
    sample4
    sample5
    sample6
    sample7
    sample8
    sample9

Output files from the sample problems can be checked against known correct output files by running the diff script.
::

    $ bash diff_output.sh

Any diffs between the files will be shown as output on the command line.

NOTE: the data in the sample folder is not suitable for real calculations and is a truncated library just for the purpose of these tests.

Running ALARA
~~~~~~~~~~~~~

Running ALARA can be done in one command line with various options.
::

    $ alara [-h] [-r] [-t <tree_filename>] [-V] [-v <n>] [<input_filename>]

Please reference the `users' guide <ALARA_Users_Guide_>`_ for specific options and more information.

Additional Notes
~~~~~~~~~~~~~~~~

In addition to building and installing the ALARA program, this package
includes an accessory program, dant2alara, and a pair of Perl scripts
for post-processing data.

**dant2alara**
is an interactive program for converting RTFLUX/ATFLUX files (from
DANTSYS and similar) to text based flux files for use in ALARA.  Since
ALARA can read RTFLUX files directly, this use may not be useful.  On
the other hand, the biological dose method requires ggamma source to
dose conversion factors from an ATFLUX type file, which can not be
handled directly by ALARA at this time.

**extract_pathways**
is a Perl script that scans an ASCII tree file and finds all the
chains/pathways that result in a given isotope.

**summary**
is a Perl script that extracts a summary of the output file, most
notably removing all results for individual isotopes and leaving all
totals.  You can also extract results for a single specific isotopes
by giving the argument "-iso" followed by the isotope in question
written as a lower case atomic symbol hyphenated with the mass
number. e.g. tritium is h-3.

More Information
~~~~~~~~~~~~~~~~

Please visit the `ALARA homepage <ALARA_Homepage_>`_ for more information regarding all of the above topics.

..  _ALARA_Homepage:    https://svalinn.github.io/ALARA
..  _ALARA_Users_Guide: https://svalinn.github.io/ALARA/usersguide/index.html
