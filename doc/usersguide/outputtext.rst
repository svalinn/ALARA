==================
ALARA Users' Guide
==================

Output File Formats
===================


===============
Standard Output
===============

Description:
============

 ALARA output is always written to the standard output of the
 system in question. The standard methods of your system
 should be used to capture this output to a file, if desired.
 A sample output is available here.

Format:
=======

 As described in the section on :doc:`command-line <commandtext.rst>`
 :doc:`[G] <glossarytext.rst>` arguments, various levels of output
 are available during the calculation. The first part of the output
 file will contain this verbose output, including confirmation
 of the input data and details of the cross-referencing and
 preprocessing of the input. 

 The second part of the output file shows details on the tree building
 process, ranging from a simple list of the root isotopes being solved
 and statistics on the size and speed of the solution, to details on
 the chain growth and truncation:doc:`[G] <glossarytext.rst>`
 calculations (depending on the verbosity:doc:`[G] <glossarytext.rst>`
 specified on the
 :doc:`command-line <commandtext.rst>`:doc:`[G] <glossarytext.rst>`). 

 The final part of the output file:doc:`[G] <glossarytext.rst>`
 are the results, as requested by the user in the input file. This
 output will include one section for each output format description
 given by the user. Each of these sections will be divided into
 blocks as shown in the following block diagram. 


+-----------------------------------------------------------------------+
|**Output Block Diagram**                                               |
|                                                                       |
| For each output description                                           |
|     For each requested response                                       |
|         For each point = (interval|zone|mixture)                      |
|             Information about point (e.g. volume, loaded mixture, etc)| 
|                 If constituent breakdown requested                    |
|                 (Optional)                                            |
|             :ref:`Table type 1` for each constituent                  |
|         :ref:`Table type 1` of total response for this point          |
|     :ref:`Table type 2` summarizing total responses for all points    |
+-----------------------------------------------------------------------+

Table Type 1
============

	The first type has a row for each isotope:doc:`[G] <glossarytext.rst>`
	produced in the problem that has a non-zero response. If 
	the constituent keyword:doc:`[G] <glossarytext.rst>` is 
	given in the output block, there will be one table for 
	each constituent, followed by a table for the total 
	mixture:doc:`[G] <glossarytext.rst>`. 

	For the isotopic results of individual mixture constituents, 
	all the values are normalized to the volume fraction of that 
	constituent. Therefore, the table represents the results 
	for a sample made up entirely of that constituent. Put 
	another way, the table represents the results normalized 
	per unit volume (or mass) containing only that constituent. 

	For most results, the table containing the total isotopic 
	results for the interval, zone:doc:`[G] <glossarytext.rst>`, 
	or mixture, is not normalized, and these values represent 
	the result for the mixture as described in the input, 
	regardless of what volume fraction if filled. However, 
	for tables showing the total waste disposal rating 
	(or clearance index) of intervals, zones, or mixtures, 
	all the values are normalized by the total volume 
	fraction of the mixture. Therefore, the table represents 
	the results for a 100% dense sample made up of 
	the same composition as the mixture. 

Table Type 2
============

	The second type of table has a row for each point in the 
	requested resolution, giving the total response at that 
	point. These results are normalized as described in the 
	previous paragraph (ie. only the waste disposal rating 
	results are normalized). 

		**Note:** Note: For reverse calculations, the
		entire structure defined above will be repeated 
		for each target isotope. 

-------------------------------

=========
Tree File
=========

Description:
============

 ALARA also optionally produces a so-called tree
 file:doc:`[G] <glossarytext.rst>` to allow some rudimentary
 pathway analysis. The tree file contains much information about
 the creation and truncation of the trees and chains used to
 calculate the transmutation and activation in the problem.

Format:
=======

 One tree will be created for each initial isotope. All the
 information given for this isotope is based on the flux chosen
 for the truncation calculations of this isotope, namely, the
 group-wise maximum flux across all the intervals in which the
 initial isotope exists. An entry for an isotope in the tree
 will look like this: 

	-(na)->h-3 - (0.00306937)

 The level of indentation indicates the rank of this
 isotope:doc:`[G] <glossarytext.rst>' in the tree. This can be
 best seen by viewing the whole file and noting the line's
 relative indentation. The information given in such an
 entry is as follows: 

	**reaction type: (na)** This indicates the reaction type(s). 

	If multiple reactions lead to this product, the reactions 
	will be separated by commas. The information indicates the 
	emitted particles only. Therefore, in this example, the 
	reaction is an (n,na) reaction. Generally, standard symbols 
	are used, such as 'n' for neutrons, 'a' for alpha particles, 
	'p','d','t' for the three isotopes of hydrogen, respectively, 
	and 'h' for helium-3. For all neutron reactions, an 
	additional '*' is used to indicate that the product is in 
	an excited isomeric state. Finally, for decay reactions 
	the symbol '*D' is used.

	**product nuclide: h-3** The product isotope's chemical 
	symbol and atomic number. 

	In cases where the product is in an isomeric state, this 
	will be followed by a letter (m,n,...) indicating which 
	isomeric state.

	**truncation mode: -** This single character indicates 
	the result of the truncation calculation at this node.

	There are four possible results as follows: 

	Result	Description

	-	This code indicates that the chain continues normally 
		because this isotope passed all the tests.
	
	*	This code indicates that only the radioactive decays 
		of the chain will be followed after this node. This 
		arises when the production does not pass the truncation 
		tolerance test, but ensures that the result includes 
		all the radioactive products. Stable products which 
		are descendants of this node may be calculated if 
		they themselves pass the ignore tolerance test.

	/	This code indicates that the chain will be fully 
		truncated at this node, and the result will include 
		this node. This arises when the node is a stable 
		isotope and does not pass the truncation tolerance 
		test, but does pass the ignore tolerance test.

	<	This code indicates that the chain will be fully 
		truncated at this node and will not be included 
		in the result. This arises when the production 
		of this nuclide does not pass either the 
		truncation or the ignore tolerance test.

	**truncation production: (0.00306937)**

	This indicates the relative production at the end of 
	operation of this nuclide from the initial isotope during 
	the truncation calculation.

	As explained in the ALARA Technical Manual], this represents 
	the total production of this nuclide during the whole problem, 
	assuming that none of it is transmuted or decays further. If 
	this production is not calculated, for example, because the 
	chain is only being followed on radioactive reactions and 
	this nuclide is stable, then this entry will be '-'.  
	
-------------------------------------

=================
Gamma Source File
=================

Description:
============

 The gamma source files created by ALARA currently has a very simple
 (if not unweildy) format. This file is only created if the
 photon_src option is used in one of the output blocks of the input
 file where the filename is also given. The spatial resolution of
 this file is that specified by the output block.

Format:
=======

 For each spatial region, there is a section for each isotope
 responsible for gamma emissions:doc:`[G] <glossarytext.rst>` and a
 section for the total gamma emissions. Each of these sections has
 a header line consisting of the isotope's identifier (chemical
 symbol and mass number) or the keyword "TOTAL", respectively.
 Within each section, there is one block of gamma source values
 for each output time, that is, one for shutdown and one for each
 cooling time. Finally, each of these blocks contains the
 group-wise gamma source values in photons per second,
 arranged in lines of 6 values per line. 
