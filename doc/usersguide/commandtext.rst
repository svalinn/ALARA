==================
ALARA Users' Guide 
==================

Command Line Options
====================


**Syntax Options:** (:ref:'Examples' below)
::

	alara [-h] [-r] [-t <tree_filename>] [-V] [-v <n>] [<input_filename>] 

Options
=======

 **Command**		**Description**

 -h			print a short help message describing the command-line

 -r			operate in a post-processing mode:doc:`[G] <glossarytext.rst>`
			
			This option is used to redo the built-in post-processing
			stage of ALARA, possibly calculating different responses than
			in the original run. ALARA creates a dump
			file:doc:`[G] <glossarytext.rst>` during operation
			which contains enough information to calculate different 
			output responses for the same problem. This option can 
			only be used after ALARA has already been successfully 
			run. To use this option, you must use the same problem 
			description (ie: the same input file) as the original 
			run, and can only change the output blocks.

			If, for example, the original ALARA run only gave results 
			for the specific activity, and the user is now interested 
			in results for decay heat and/or a consituent break-down. 
			The user could change the output block of the
			:doc:`input file <inputtext.rst>` (or add a new output
			block) to include the newly desired results and then 
			rerun ALARA using this restart option. 

 -t <tree_filename>	set the filename for the tree file

			This option defines the name of the optional tree 
			file:doc:`[G] <glossarytext.rst>` to be generated during 
			the ALARA run. If it already exists, this file will be 
			overwritten. For more information on tree files, see the 
			Users' Guide section devoted to :doc:`output files <outputtext.rst>`
			:doc:`[G] <glossarytext.rst>`. 

 -v			show version string 

			This option will show the current version string and stop!

 -v <n>			set the verbosity level of the output

			This option is used to determine how much output is generated 
			by ALARA. Normally, ALARA will only generate output for the 
			results requested in an output block. Using this option, you 
			can increase the level of output given during the calculation 
			itself. The level is set using an interger argument, <n>. 

			Verbosity Level Options:

			+---------+-----------------------------------------------------------+
			|    1    |Shows top-level program phase (e.g. input/solution/output) |
			+---------+-----------------------------------------------------------+
			|    2    |Shows second level program phase (e.g. reading             |
			|	  |input/cross-checking input/preprocessing input ...)        |
			+---------+-----------------------------------------------------------+
			|    3    |**This is the recommended level.** Shows fine level        |
			|         |program phase, including basic echoing of input and basic  |
			|	  |statistics on the solution status.                         |
			+---------+-----------------------------------------------------------+
			|   4-6   |Increased verbosity:doc:`[G] <glossarytext.rst>` gives     |
			|	  |more information in input processing and more solution     |
			|	  |statistics. During input processing, this includes         |
			|	  |expansion of materials and calculation of interval volumes,|
			|	  |and during the solution, this includes information about   |
			|	  |each node being added to the trees, their library          |
			|	  |status and truncation status.                              |
			+---------+-----------------------------------------------------------+

<input_filename>	define the :doc:`input file <inputtext.rst>` :doc:`[G] <glossarytext.rst>`

			This option defines which :doc:`input file <inputtext.rst>` :doc:`[G] <glossarytext.rst>`
			will be used. If not name is given, the input will be read from stdin. 
				
.. _Examples:
		
Examples
========


 To start alara with no tree file and no verbosity, with an input file names case1: 

	alara case1 


 To start alara with an input file named case2, moderate verbosoty and creating a tree file named tree2: 

	alara -t tree2 -v 3 case2 

