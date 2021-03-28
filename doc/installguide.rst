================
Installing ALARA
================

Requirements
============

ALARA is written primarily in C++ with one or two FORTRAN77 routines. All
development is done with the GNU C++ and FORTRAN compilers and it has been
tested and is known to work on Linux and Solaris.

The following pieces of software are required in order to build ALARA:

* C++ compiler
* Fortran compiler
* CMake

Installation Process
====================

Here are some commands that can be used to build ALARA.
::

    $ INSTALL_PREFIX=${PWD}
    $ git clone https://github.com/svalinn/ALARA
    $ mkdir bld
    $ cd bld
    $ cmake ../ALARA -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
    $ make
    $ make install

Installation Options
====================

Some common CMake options are listed here:

``-DCMAKE_INSTALL_PREFIX`` allows you to change the location for the
installation of ALARA and its accompanying files. The next section describes the
different files that are installed and the directories in which they are
installed.

``-DCMAKE_CXX_COMPILER`` and ``-DCMAKE_Fortran_COMPILER``
allow you to specify custom locations of the C++ and Fortran compilers you want
to use to compile ALARA. If left out, CMake will search for default compilers.

Installed files and directories
===============================

When installing ALARA, a number of files are installed in a number of different
places. This section summarizes these directories which are all subdirectories
of the ``CMAKE_INSTALL_PREFIX`` directory described above.

**bin**

This directory contains the installed executables. The following files are
installed to this directory:

* ``alara`` - the main application
* ``dant2alara`` - a utility for converting DANTSYS rtflux/atflux files to a
  format suitable for ALARA
* ``summary`` - a Perl utility for extracting summaries of ALARA output files
* ``extract_pathways`` - a Perl utility for extracting pathways from ALARA tree
  files

**data**

This directory contains a number of machine-independent files (generally ASCII
text files) which contain data used by ALARA. Descriptions of these files can be
found in the ``Data_list.txt`` file which is located in this directory.

**sample**

This directory contains a number of sample ALARA problems for testing purposes.
