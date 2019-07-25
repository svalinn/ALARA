=================
Input File Syntax
=================

Input File Description:
=======================

 The input file for ALARA has been designed to ensure that
 the input information is easy to understand, edit and comment.
 This is possible by using a very free format permitting
 comments, blank lines, inclusion of other files, and
 arbitrary ordering of the input information. After reading
 the full input file, ALARA performs various cross-checks and
 cross-references to ensure that the input data is
 self-consistent. It then goes on to pre-process the data for
 the calculation. Every attempt has been made to give
 useful :doc:`error messages <errortext>` when the data
 is not consistent.

General Information:
====================

 There are many possible input block types. These blocks
 can appear in any order and many blocks can occur more
 than once, if at all. Each of the input blocks includes a
 link to a sample block, as it exists in a complete
 sample input file.

 In the following list, input blocks indicated in bold
 are required at least once in every input
 file :doc:`[G] <glossarytext>`:

 +--------------------------+---------------------------+--------------------------+
 |Geometry & Materials      |Flux Schedules & Chain -   |Output & Files            |
 |                          |Building                   |                          |
 +--------------------------+---------------------------+--------------------------+
 |geometry                  |flux                       |cooling                   |
 +--------------------------+---------------------------+--------------------------+
 |dimension[1]              |spatial_norm               |output                    |
 +--------------------------+---------------------------+--------------------------+
 |major_radius              |schedule                   |material_lib              |
 +--------------------------+---------------------------+--------------------------+
 |minor radius              |pulsehistory               |element_lib               |
 +--------------------------+---------------------------+--------------------------+
 |volumes[1]                |truncation                 |dump_file                 |
 +--------------------------+---------------------------+--------------------------+
 |mat_loading               |impurity                   |data_library              |
 +--------------------------+---------------------------+--------------------------+
 |mixture                   |ignore                     |convert_lib               |
 +--------------------------+---------------------------+--------------------------+
 |solve_zones               |ref_flux_type              |                          |
 +--------------------------+---------------------------+--------------------------+
 |skip_zones                |                           |                          |
 +--------------------------+---------------------------+--------------------------+


General Input Notes
===================

 1. **Dimension/Volume:** If “[1]” follows an input
    block name, either the dimension :doc:`[G] <glossarytext>`
    or the volume is required, but it will generate an
    error if both are included. 

 2. **Input Blocks:** Not all input blocks are required,
    with some being unnecessary for certain problems. Input
    blocks indicated in bold are required at least once in
    every input file :doc:`[G] <glossarytext>`.

    There are also some input blocks which are incompatible
    with each other. While superfluous input blocks may go
    unnoticed (there are occasional warnings), incompatible
    input blocks will create an error.

 3. **Naming Input Data:** Most input blocks allow the user
    to define their own symbolic names for cross-referencing
    the various input data. Any string of characters can be
    used as long as its does not contain any
    whitespace :doc:`[G] <glossarytext>` (spaces, tabs,
    new-lines, etc.).

    It is considered dangerous, however, to use a
    keyword :doc:`[G] <glossarytext>` as a symbolic name.
    If the input file is correct, it will function properly,
    but if there are errors in the input file, the usage of
    keywords as symbolic names may make the error message
    irrelevant. The keywords include those listed in the
    above list and the keyword "end". 

 4. **Keyword “End”:** While many input blocks of fixed
    length require nothing to indicate the end of the
    block, some blocks have a variable length and require
    the keyword "end" to terminate the block. 

 5. **Time Based Input:** Some input elements represent
    times and can be defined in a number of different units.
    When this is the case, the floating point time value
    should be followed by a single character representing
    the following units: 

    Sample Input:   10 d

|    [s]econd
|    [m]inute             60 seconds
|    [h]our               60 minutes
|    [d]ay                24 hours
|    [w]eek               7 days
|    [y]ear               52 weeks
|    [c]entury            100 years


 6. **Input File Inclusion:** One input file can be
    included in another with the #include directive, similar
    to the C programming language. Any number of files can
    be included. Included files can also contain directives
    to include other files. The only restriction is that
    the inclusion must not occur within an input block! 

 7. **Commenting:** All other lines in which the first
    non-space character is the pound sign (or number
    sign) (#) are considered as comments. Comments can
    also be used after any single word input (an input
    value with no whitespace :doc:`[G] <glossarytext>`)
    by using the same comment character (#). Such
    comments extend to the end of the current line.
    Blank lines are permitted anywhere in the input
    file :doc:`[G] <glossarytext>`. 

 8. **Length Units:** Centimeters should be used for all
    length units.

----------------------

=================
Input Block Types
=================

Section I: Geometry & Materials
===============================

	**Name:** geometry (required once) 

		**Description:** This required input block is
		only necessary when defining a geometry using 
		the dimension :doc:`[G] <glossarytext>` input 
		block, but may always be included. It should 
		only occur once. 

		**Syntax:**
		::

			geometry <OPTION>


		**Option Description:**

|			point :doc:`[G] <glossarytext>` --
|			rectangular :doc:`[G] <glossarytext>` --
|			cylindrical :doc:`[G] <glossarytext>` --
|			spherical :doc:`[G] <glossarytext>` --
|			torus :doc:`[G] <glossarytext>` --

		**Sample Input:** geometry point

		**Notes:** This input block should not be terminated. 
		If using the dimension input block to define the geometry 
		and the type is torus :doc:`[G] <glossarytext>` , the 
		major_radius :doc:`[G] <glossarytext>` input block is 
		required and the minor_radius :doc:`[G] <glossarytext>`
		block may also be required. 

------------------

	**Name:** dimension (required [1]: once [1d], twice [2d], thrice [3d])

		**Description:** This input block is used to define the 
		geometry layout, and should be included once for each 
		dimension :doc:`[G] <glossarytext>` needed in the problem. 

		**Syntax:** 
		::

			dimension <OPTION> <LOC0>
			<#INTS1> <LOC1>
			<#INTS2> <LOC2>
			.
			.
			.
			<#INTSN><LOCN>
			end

		**Option Description:** The dimension block's first element 
		indicates which dimension :doc:`[G] <glossarytext>` is being 
		defined and should be one of the following: 

			x --
			y --
			z --
			r --
			theta --
			phi --

		**Sample Input:** The dimension block's next element is the 
		first zone's lower boundary, expressed as a floating point 
		number :doc:`[G] <glossarytext>`. This is followed by a list 
		of pairs, one pair for each zone: an integer specifying the 
		number of intervals in this zone in this dimension and a 
		floating point number indicating the zone's upper boundary. 
		This list is terminated with the end keyword. 
		::

			dimension x 0.0
			1.0
			10 2.5
			5 5
			end

		**Notes:** ALARA will check to ensure that only dimensions 
		relevant to the defined geometry are included. For example, 
		defining the 'x' dimension in a spherical :doc:`[G] <glossarytext>`
		problem will generate an error. Since this method of defining 
		the geometry calculates the fine mesh intervals' :doc:`[G] <glossarytext>`
		zone membership and volume from the dimension data, it is 
		incompatible with the volumes input block. Including 
		both will generate an error message.

-------------------------

	**Name:** major_radius and minor_radius (required once [each] for geometry torus) 

		**Description:** These two input blocks are used to define 
		the major and minor radii :doc:`[G] <glossarytext>` of toroidal 
		geometries. They are only needed in defining a 
		toroidal :doc:`[G] <glossarytext>` geometry with dimension 
		input blocks, and each should only be included once. 
		Furthermore, if the minor radius dimension is defined with 
		a dimension block, the minor_radius input block is not 
		required. In both cases, these input blocks have a fixed 
		size, with a single argument specifying the radius as 
		a floating point number.

		**Syntax:**
		::

			Major_radius <value>
			
		**Sample Input:**
		::

			Major radius 1.25
			
		**Notes:**

-----------------------

	**Name:** volumes (required [1] once) 

		**Description:** This input block is used to define the 
		fine mesh intervals' :doc:`[G] <glossarytext>` volumes 
		and zone membership. 

		**Syntax:**
		::

			Volumes
			<VOL of INT1> <INT Name>
			.
			.
			<VOL of INT#> <INT Name>
			end

		This input block should be a list of pairs, one pair 
		for each interval. Each pair consists of a floating 
		point :doc:`[G] <glossarytext>` value for the volume 
		of that interval and the symbolic name of the zone 
		containing that interval. These symbolic names 
		should correspond with the symbolic names given 
		to the zones in the mat_loading input block. This 
		list must be terminated with the 
		keyword :doc:`[G] <glossarytext>` end. 

		**Sample Input:**
		::
		
			volumes
			0.5 first_wall
			0.75 first_wall
			1.2 shield
			end

		**Notes:** This block can be used instead of the 
		dimension method of defining the geometry. If 
		both are used, an error will result. This block 
		should only occur once. Multiple occurrences will 
		result in undefined behavior.

------------------

	**Name:** mat_loading (required once) 

		**Description:** This input block is used to 
		indicate which mixtures are contained in each 
		zone :doc:`[G] <glossarytext>`. This block is 
		a list with one pair of entries for every zone. 
		Each pair consists of a symbolic name for the 
		zone and a symbolic name for the mixture 
		:doc:`[G] <glossarytext>` contained in that 
		zone. This list is terminated by the keyword 
		end. This block should only occur once. 
		Multiple occurrences will result in undefined 
		behavior. 

		**Syntax:**
		::

			mat_loading
			<zone1name> <mix1name>
			<zone2name> <mix2name>
			.
			.
			<zone#name> <mix#name>
			end

		**Sample Input:**
		::

			mat_loading
			
			end

		**Notes:** If the geometry is defined using the 
		dimension input blocks, the number of zones 
		:doc:`[G] <glossarytext>` defined here must match 
		the number of zones defined in the dimension 
		blocks exactly; if not, an error results. If 
		the volumes method is used to define the geometry, 
		this block uniquely determines the number of zones.
		The symbolic name for the mixture must match one 
		of the mixture definitions exactly, or be the 
		keyword 'void', indicating that 
		this zone is empty of material. 

------------------------

	**Name:** mixture (required: once per defined mixture) 

		**Description:** This kind of block is used to 
		define the composition of a mixture. This block 
		can occur as many times as necessary to define 
		all the mixture compositions in the problem. 
		Any mixtures that are defined, but not used in 
		the problem will generate a warning and be 
		removed from the list of mixtures. 

		**Syntax:**
		::

			mixture <mixname>
			<OPTION1>
			<OPTION2>
			.
			.
			.
			<OPTION#>
			end

		The first element of a mixture block is the symbolic 
		name used to refer to this mixture elsewhere 
		in the input file. Following this is a list of 
		entries with one entry for each mixture constituent. 
		The list must be terminated with the keyword 'end'. 
		The first element of each entry describes the 
		type of that constituent and should be one of: 

		**Option Description:**

		The remaining elements in each entry are interpreted 
		as follows, based on this first element: 

			**material**

			This entry has three additional elements. The 
			second element in this entry is the symbolic 
			name of a material definition existing in 
			the material library. The third element is a 
			floating point value representing the relative 
			density of this material, based on the density 
			given in the material library. The final 
			element is a floating point :doc:`[G] <glossarytext>`
			value representing the volume fraction of 
			this material in this mixture. Both of the 
			last two values are typically between 0 and 1.

			The purpose of these values is quite distinct 
			and should correspond to the physical system 
			being modelled. Their proper use will ensure 
			that the detailed output is correctly normalized. 
			For example, if a user wishes to model a region 
			containing 50% SiC, where the SiC has been 
			manufactured at 95% of theoretical density. The 
			relative density element should by 0.95 and 
			the volume fraction element should be 0.50. 

			**element**

			This entry has three additional elements. The 
			second element in this entry is the element's 
			modified chemical symbol :doc:`[G] <glossarytext>`. 
			This element will be expanded into a list of 
			isotopes :doc:`[G] <glossarytext>` using the 
			abundances found in the element library for 
			that modified chemcial symbol. A modified 
			chemical symbol :doc:`[G] <glossarytext>` has 
			the format ''ZZ:XXXXXX...'', where ZZ is the 
			standard chemical symbol, and the string
			XXXXXX... allows for isotopic 
			abundances :doc:`[G] <glossarytext>` different 
			from natural abundances :doc:`[G] <glossarytext>`.

		The final two elements of this section are identical to 
		the final two elements of the material type entry, 
		and should be interpreted in the same way.  

			**like**

			This type of entry has two additional elements 
			and is provided as a convenience and indicates 
			that this constituent is like another user-defined 
			mixture :doc:`[G] <glossarytext>`, with a 
			potentially different density. The second element 
			of this entry is the symbolic name of another 
			mixture definition. If the other mixture 
			definition is not found, an error will result. 
			The entry's final element is a relative density, 
			used to normalize the density as defined in 
			that mixture's own definition. This might be 
			used when a user-defined mixture makes up part 
			of another mixture. [Hint: it is permissible to 
			define a mixture that is not used in any zones, 
			but only used as part of another mixture.] 

			**target**

			This type of entry is used to initiate a reverse 
			calculation (see the ALARA Technical Manual) 
			and define the target isotopes :doc:`[G] <glossarytext>`
			for the reverse calculation. The user can 
			define an arbitrary number of target isotopes. 
			The second element of this entry is one of the 
			keywords element or isotope, indicating what kind 
			of target this is. The final element is the symbolic 
			name of either the element or isotope. For isotopes, 
			the symbolic name is in the format ZZ-AAA, where ZZ 
			is the chemical symbol :doc:`[G] <glossarytext>` and 
			AAA is the mass number. There are no elements 
			representing relative densities or volume fractions. 
			If a target is of type element, the element will be 
			expanded using the element library to create a list 
			of isotopes, but their atomic abundance is irrelevant. 

		**Sample Input:**

		**Notes:** Even if a target is defined in only one mixture, 
		it will cause the whole problem to be run as a reverse 
		problem. There is therefore little purpose in having mixture 
		definitions without targets (such as in this example). 

-------------------

	**Name:** solve_zones (optional once) 

		**Description:** This optional input block allows the 
		user to limit which zones are being solved in a given 
		calculation. It is common for a user to create a
		single complete input file describing the entire 
		geometry/composition, and want to include only certain 
		parts of the geometry/composition for particular cases. 

		**Syntax:**
		::

			solve_zones
			<zone1name>
			<zone2name>
			.
			.
			<zone#name>
			end

		This input consists of a list of symbolic names of 
		the zones that are to be solved in this case. These 
		symbolic names should correspond with the symbolic 
		names given to the zones in the mat_loading input 
		block. This list must be terminated with the 
		keyword end.

		**Sample Input:**

		**Notes:**

-----------------------

	**Name:** skip_zones (optional once) 

		**Description:** This optional input block allows 
		the user to limit which zones are being solved in 
		a given calculation (see solve_zones). It is 
		common for a user to create a single complete 
		input file :doc:`[G] <glossarytext>` describing 
		the entire geometry/composition, and want to 
		exclude certain parts of the geometry/composition 
		for particular cases. 

		**Syntax:**
		::

			skip_zones
			<zone1name>
			<zone2name>
			.
			.
			<zone#name>
			end

		This input consists of a list of symbolic names 
		of the zones that are NOT to be solved in this case. 
		These symbolic names should correspond with the 
		symbolic names given to the zones in the 
		mat_loading input block. This list must be 
		terminated with the keyword end. 

		**Sample Input:**

		**Notes:**

------------------------------

Section II: Flux Schedules & Chain-building
===========================================

	**Name:** flux (required: once per defined flux) 

		**Description:** This input block defines a set 
		of flux spectra :doc:`[G] <glossarytext>`. 

		**Syntax:**

		The first element of this block is a symbolic name, 
		used to refer to this flux spectra definition. The 
		other elements of this block are a filename, a 
		floating point scalar normalization 
		:doc:`[G] <glossarytext>`, an integer skip value 
		(see below), and flux type indicator string, 
		respectively. 

		The flux filename should indicate which file contains 
		this flux information, including path information 
		appropriate to find the file from the directory in 
		which ALARA will be run. The flux file itself 
		contains a simple list of group fluxes for each of 
		the fine mesh intervals :doc:`[G] <glossarytext>` 
		defined in the problem. The number of groups for 
		each interval and the order of those groups is 
		determined entirely by the data library being used. 
		ALARA places no restrictions or assumptions on these. 
		Blank lines are ignored in the input, and may be 
		used to separate the entries for each interval. 

		The scalar normalization permits uniform flux 
		scaling at all spatial points (as opposed to the 
		spatial_norm information in the next section). All 
		groups of all fluxes in this definition will be 
		multiplied by this value. 

		The skip value indicates how many N-group flux 
		entries to skip in this file before reading the 
		first flux. This permits the user to have one file 
		with many different flux spectra. For example, if 
		the schedule requires two different flux spectra 
		for N different fine mesh points, the data for the 
		first one may be at the beginning of the file, 
		with a skip of 0, while the data for the second 
		flux definition would be after these first fluxes, 
		with a skip of N. 

		The last element is a character string indicating 
		the flux file's format. Currently the only 
		supported format is default. The default flux 
		file format consists of one list of group fluxes 
		per spatial point. There are no other entries and 
		this can be freely formatted, although comments 
		are not permitted. 

		[Hint: Different flux definitions might use exactly 
		the same flux values (same flux file and skip value) 
		but a different scaling value.] 

		**Sample Input:**

		**Notes:**

		Since different parts of the irradiation 
		history :doc:`[G] <glossarytext>` can have different 
		flux spectra, this block may occur as many times as 
		necessary to represent all the different necessary 
		flux definitions. 

-----------------------

	**Name:** spatial_norm (optional once) 

		**Description:** This input block allows the user 
		to specify a scalar flux normalization for each fine 
		mesh interval :doc:`[G] <glossarytext>`, such as 
		might be required to re-normalize the results of 
		a transport calculation on an approximated geometry.

		**Syntax:**

		This block consists of a list of floating point 
		normalization values, one value for each interval, 
		and requires the end keyword to terminate the list. 

		**Sample Input:**

		**Notes:**

		The number of normalizations must be at least as 
		many as the number of defined intervals, regardless 
		of how the intervals are defined (dimension vs. 
		volumes). If there are too few, an error will 
		result; if there are too many, a warning will result. 

		[Hint: if these values are purely a function of 
		problem geometry, and not mixture composition, it 
		is possible that many problems have the same 
		spatial normalization. Put this data in a separate 
		file and #include it when you need it.] 

-------------------------

	**Name:** schedule (required: once per defined schedule) 

		**Description:** This kind of block is used to 
		define a single schedule in the full 
		irradiation history hierarchy. 

		**Syntax:**

		The first element in this input block is a symbolic 
		name by which this schedule can be referred to. 
		Following this is a list of items occurring in this 
		schedule. There are two possible types for each 
		item, and their may be an arbitrary list of items 
		in a schedule. This list must be 
		terminated with the keyword 'end'.

		The first type of item is a simple pulse and the 
		entries for this kind of item are a floating point 
		operating time, a single character defining the 
		units of that operating time, a symbolic flux name, 
		a symbolic pulsing definition name, a floating point 
		post-item delay time, and a single character 
		defining the units of that delay time. 

		The second type of item is a sub-schedule and the 
		entries for this kind of item are a symbolic name for 
		the sub-schedule, a symbolic pulsing definition name, 
		a floating point post-item delay time, and a single 
		character defining the units of that delay time. 

		In both cases, if the symbolically named items 
		(flux, pulsing definition, or schedule) are not 
		found during cross-referencing, an error results. 

		**Sample Input:**

		**Notes:**

		Since the hierarchy may be composed of many schedules, 
		this block might occur many times. Since schedules 
		can become complicated, a tutorial is available 
		for forming complex schedules. 

----------------------------

	**Name:** pulsehistory (required: once per defined history) 

		**Description:** This kind of input block defines 
		the multi-level pulsing histories referenced 
		in the schedule definitions.

		**Syntax:**
		::

			pulsehistory <name>
			.
			.
			end

		The first element of each block is a symbolic name 
		for referring to this pulsing schedule. Following this 
		is a list of pulsing level definition triplets, 
		each consisting of an integer number of pulses, a 
		floating point delay time between pulses, and a 
		single character defining the units of that delay 
		time. Since an arbitrary number of pulsing levels 
		is allowed, this list must be terminated with 
		the keyword 'end'. 

		**Notes:**

		The tutorial on forming complex schedules includes 
		more details on pulsing histories. Since many 
		different pulsing histories may be used throughout 
		the hierarchy of schedules, this block may occur 
		many times. 

---------------------------

	**Name:** truncation (required once) 

		**Description:** This fixed sized input block 
		defines the primary parameter used in 
		truncating :doc:`[G] <glossarytext>` the activation 
		trees. See the ALARA Technical Manual for a 
		detailed discussion of the tree truncation issue.

		**Syntax:**
		::

			truncation <tol_value>

		The only element of this block is the truncation 
		:doc:`[G] <glossarytext>` tolerance. 

		**Sample Input:**
		::
	
			truncation .001

		**Notes:**

		When testing the relative atom loss (or relative 
		production in reverse calculations), any value 
		higher than the truncation tolerance will 
		result in continuing the tree while lower 
		values will result in truncation. 

-------------------------

	**Name:** impurity (optional once) 

		**Description:** This fixed sized input block 
		defines the parameters used to treat initial 
		isotopes :doc:`[G] <glossarytext>` as impurities. 
		This feature allows the user to build shorter 
		chains for impurities, since their contributions 
		tend to be less significant. This can make 
		ALARA run much faster when impurities with 
		very large cross-sections are present. 

		**Syntax:**
		::

			impurity
			<threshold>
			<tolerance>

		The first element of this block is a floating 
		point number defining the threshold for treating 
		an isotope as an impurity. This value is a 
		relative concentration within a mixture. 
		Therefore, if the user wishes to treat all 
		isotopes which make up less than 10 atom-parts-
		per-million [appm] as impurities, they would 
		enter '1e-5' for this element. The remaining 
		element is the truncation tolerance to be used 
		for these impurities. They have the same 
		definition as given in the description of the 
		truncation input block. 

		**Sample Input:**
		::

			impurity
			2e-5
			3e-8

		**Notes:**

		To make effective use of this input block, 
		the value given for tolerance should be 
		orders of magnitude larger than the value 
		given in the truncation threshold. 

------------------------

	**Name:** ignore (optional once) 

		**Description:** This optional fixed sized input 
		block defines an additional parameter used 
		in truncating the activation trees. 

		**Syntax:**
		::

			ignore
			<tolerance>

		The only element of this block is the relative 
		ignore tolerance. When truncating chains, if the 
		value is also lower than the absolute ignore 
		tolerance, that node is completely ignored. The 
		absolute ignore tolerance is calculated by 
		multiplying by the truncation tolerance (or the 
		impurity truncation tolerance, as 
		appropriate) by this value. 

		**Sample Input:**
		::

			ignore
			10e-3

		**Notes:**

		See the ALARA Technical Manual for a detailed 
		discussion of the tree truncation issue. When 
		this input is not included, a relative ignore 
		tolerance of 10-2 is used - that is, a relative 
		production 100 times lower than the truncation 
		tolerance.

------------------

	**Name:** ref_flux_type (optional once) 

		**Description:** This optional fixed sized 
		input block defines the type of reference flux to use. 

		**Syntax:**
		::

			ref_flux_type <OPTION>

		**Option Description:**

		This input block takes a single argument, 
		which must be one of the following: 

		|	max -- refers to the default group-wise maximum flux
		|	volume_avg -- refers to a volume weighted average flux

		**Sample Input:**
		::

			ref_flux_type max

		**Notes:**

		In both cases, the comparison/averaging takes place 
		over all the intervals which contain a given root 
		isotope :doc:`[G] <glossarytext>`, not just over 
		a single zone, component, or material loading. 

--------------------------------------

Section III: Output & Files
===========================

	**Name:** cooling (optional once) 

		**Description:** This input block is used to define the 
		after-shutdown cooling times :doc:`[G] <glossarytext>` 
		at which the problem will be solved. 

		**Syntax:**
		::

			cooling
			<time1 [unit]>
			<time2 [unit]>
			.
			.
			.
			<time# [unit]>
			end

		This block is simply a list of times, where each time 
		consists of a floating point time followed by a single 
		character defining the time's units. Since an arbitrary 
		number of cooling times :doc:`[G] <glossarytext>` can 
		be solved, this list must be terminated with the 
		keyword 'end'. 

		**Sample Input:**
		::

			cooling
			<.01 s>
			<10 s>
			.
			.
			.
			<30 s>
			end

		**Notes:**

		 Multiple occurrences will result in undefined behavior.

-----------------

	**Name:** output (optional: once per required output definiton) 

		This kind of input block allows the user to define the 
		output's resolution and format. The first element of 
		an output format block indicates the resolution and 
		should be one of: 

			interval | zone | mixture

		This is followed by a list of output types and 
		modifiers described in the following table:


+-----------------+-----------+---------------------------------------------------------+
|keyword          |value      |function                                                 |
+-----------------+-----------+---------------------------------------------------------+
|constituent      |--         |generate a constituent breakdown in addition to total    |
|                 |           |response                                                 |
+-----------------+-----------+---------------------------------------------------------+
|units            |[units]    |define the units to be used for this output block        |
+-----------------+-----------+---------------------------------------------------------+
|number_density   |--         |number density result of all produced isotopes           |
+-----------------+-----------+---------------------------------------------------------+
|specific_activity|--         |specific activity of all radioactive isotopes            |
+-----------------+-----------+---------------------------------------------------------+
|total_heat       |--         |total decay heat                                         |
+-----------------+-----------+---------------------------------------------------------+
|alpha_heat       |--         |total alpha heating                                      |
+-----------------+-----------+---------------------------------------------------------+
|beta_heat        |--         |total beta heating                                       |
+-----------------+-----------+---------------------------------------------------------+
|gamma_heat       |--         |total gamma heating                                      |
+-----------------+-----------+---------------------------------------------------------+
|photon_source    |[see below]|gamma source distribution with user-defined group        |
|                 |           |structure                                                |
+-----------------+-----------+---------------------------------------------------------+
|folded_dose      |determined |fold the gamma source with a known adjoint gamma flux    |
|                 |by dose    |response for a total dose                                |
|                 |response   |                                                         |
+-----------------+-----------+---------------------------------------------------------+
|wdr              |[filename] |waste disposal rating/clearance                          |
+-----------------+-----------+---------------------------------------------------------+


		The units output modifier is used to perform unit 
		conversion on the output and requires two additional 
		text parameters. The first parameter is defines the 
		units for specific activity and related output 
		types and has two possibilities: 

			Ci | Bq

		representing "Curies" and "Bequerel" respectively. 
		The second parameter defines the units for normalization 
		(typically volumetric vs. mass). This parameter has 
		five possibilities: 

			cm3 | m3 | g | kg | volume_integrated

		The first four of these are self-evident, giving 
		different volumetric and mass normalizations. The 
		fifth option allows the calculation of total volume 
		integrated inventories, rather than volume/mass 
		normalized results. 

		The photon_src output modifier is used to generate a 
		separate file with the gamma source distribution. For 
		more information on gamma source files, see the Users' 
		Guide section devoted to :doc:`output files <outputtext>`. 
		The first additional parameter is a string representing 
		the name of the ALARA v2.x binary gamma library. The 
		extension ".gam" will be added to the path/filename 
		given here. The next parameter is a string representing 
		the the filename where the gamma source information 
		should be stored. This is followed by an integer 
		parameter representing the number of gamma groups to be 
		used for this photon source. Finally, one floating 
		point value should be given for the upper bound of 
		each gamma group (the lower bound of the lowest energy 
		group is always 0) in units of eV. These are given 
		in order of INCREASING energy.

		The folded_dose output modifer requires the following paramters: 

		*  the name of the ALARA v2.x gamma library 
		*  the volume of the detector volume 
		*  the name of the adjoint flux file 
		*  the number of photon groups 
		*  photon group boundaries from highest to lowest 

		The number of groups and group boundary values must 
		be consistent with the adjoint flux file. No automatic 
		test for consistency is performed so inconsistent 
		values will not be reported and erroneous results 
		will occur.

		The wdr output modifier requires an additional text 
		string parameter representing the filename to use for 
		calculating the waste disposal rating :doc:`[G] <glossarytext>`
		or clearance limits. A detailed description of the WDR 
		file is available here. To calculate the WDR based on 
		different standards, simply repeat this modifier within 
		a single output block, using different WDR filenames 
		each time. Be sure that the units modifier defines 
		units that correspond to those in the WDR file. 

		See the section on Output File Formats for detailed 
		for information on interpreting the output files 
		generated by ALARA. 

-------------------

	**Name:** material_lib and element_lib (required once [each]) 

		**Description:** These two input blocks are used to specify 
		the libraries to be used for looking up the definitions 
		of materials and elements when they are given as 
		mixture constituents.

		**Syntax:** Each block has a single element consisting 
		of the filename to be used in each case, including 
		appropriate path information to find that file from 
		the directory where ALARA is being run.

		**Sample Input:**

		**Notes:**

		For more information on the format of these libraries, 
		see the section on :doc:`Support Files <support>`. 

---------------------

	**Name:** dump_file (optional once) 

		**Description:** This input block defines the filename 
		to use for the binary data dump produced during a run 
		of ALARA. This is currently used to store the 
		intermediate results during the calculation, and will 
		be extended in the future to allow sophisticated 
		post-processing of the data. This filename should be 
		a valid name for a new file, including path information 
		appropriate for the directory where ALARA will be run. 

		**Syntax:**

		**Sample Input:**

		**Notes:**

		If the dump file already exists, it will be overwritten 
		with no warning. If this input block is omitted, the 
		default name 'alara.dump' will be used. 

-----------------------

	 **Name:** data_library 

		**Description:** This input block is used to define 
		the type and location of the nuclear data library. 

		**Syntax:**
		::

			data_library <OPTION>

		The first element of this block is a character string 
		defining the type of library. The subsequent elements 
		indicate the file's location. 

		**Option Description:** Currently accepted library 
		types are: 

			alaralib - Standard ALARA v2.x binary library 

				This library type requires a single filename 
				indicating the library's location.

			adjlib - Standard ALARA v2.x reverse library 

				This library type requires a single filename 
				indicating the library's location.

			eaflib - Data library following EAF formatting 
			conventions (ENDF/B). 

				This library type requires two filenames, the 
				transmutation library and the decay library, 
				respectively. These libraries will be read and 
				processed, creating an ALARA v2.x binary library 
				with the name 'alarabin' for use in subsequent 
				calculations. Alternatively, this library could 
				be converted to an ALARA v2.x binary library 
				as a separate process using the 
				convert_lib function.

		**Sample Input:**

		**Notes:**

		For both types of ALARA v2.x library, the extension ".lib" 
		will be added to the filename indicated in this input 
		block. Otherwise, all filenames should include 
		appropriate path information to find the file from the 
		directory in which ALARA will be run. 

-------------------------

	**Name:** convert_lib 

		This input block is used to convert library formats. If 
		this input block is included, ALARA will stop immediately 
		after converting the library (ie. it should not be used 
		as part of a normal ALARA input file). 

		The first two elements of this input block indicated 
		the original library format and the desired new 
		format, respectively. The following values are allowable: 

|		alaralib - Standard ALARA v2.x binary library 
|		adjlib - Standard ALARA v2.x reverse library 
|		eaflib - Data library following EAF formatting 
		conventions (ENDF/B). 

		The number and nature of the subsequent elements depend 
		on these first two elements, but are divided into two 
		sections. The first section is dependent on the first 
		element (the original library format) and the second 
		section is dependent on the second element (the 
		desired library format): 

		*alaralib*

		This section has a single element, the base name of the 
		ALARA v2.x libraries to be generated. Four (4) files 
		will be created with the following extensions: 

|		.lib - the binary reaction library 
|		.idx - a copy of the reaction index which is included 
		in the binary reaction library 
|		.gam - the binary gamma source library (coming soon!) 
|		.gdx - a copy of the gamma source index which is 
		included in the binary gamma source library 

		*adjlib*

		This section has a single element, the base name of the 
		ALARA v2.x reverse libraries to be generated. Two (2) 
		files will be created with the following extensions: 

|		.lib - the binary reverse reaction library 
|		.idx - a copy of the reverse reaction index which is 
		included in the binary reverse reaction library

		*eaflib*

		This section requires two elements, the filenames of 
		the multi-group cross-section library and the 
		decay/gamma library, respectively. 

		[Note: It current only supports the conversion from 
		EAF formatted libraries to ALARA v2.x binary libraries.]
