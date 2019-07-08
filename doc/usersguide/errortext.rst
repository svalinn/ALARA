==================
ALARA Users' Guide
==================

Error Messages
==============


Error Message Types
===================

	:ref:`Command-line Parsing`
	:ref:`Input Phase`

		:ref:`Read Input File`
		:ref:`Input Checking`
		:ref:`Input Cross-referencing`

	:ref:`Data Library Errors`
	:ref:`Programming Errors`

Example Error Message:
======================

| Error name/number:     130: Invalid dimension:doc:`[G] <glossarytext.rst>` type: <string> 
| Description:           The type of dimension:doc:`[G] <glossarytext.rst>`, string,
		         declared in the dimension block is not supported.

Command-line Parsing
====================

**0: Option <string> is not implemented yet.**
	
    An unsupported command-line option was specified: string.

**1: Only one input filename can be specified: <string>.**

    There appears to be more than one input filename on the
    command-line:doc:`[G] <glossarytext.rst>`. This may be 
    caused by an error in the other command line options,
    or a missing option.


Input Phase
===========

 Note: All error messages which occur during the input phase
 may not report the accurate cause of the error. If there is
 an error in the input file, ALARA may not immediately
 recognize the error and then report an error during some
 later input block. This is particularly true during the
 first step, reading the input file:doc:`[G] <glossarytext.rst>`
 itself.

Read Input File
===============

**100: Invalid token in input file: <string>**

    There is an error in the input file causing it to read
    an invalid keyword`[G] <glossarytext.rst>`.

**101: Unable to open included file: '<string>'.**

    The file string included in one of the input files
    can not be opened.

**110: Unable to open material library: <string>**

    The file string specified in the material_lib input
    block cannot be opened.

**111: Unable to open element library: <string>**

    The file string specified in the element_lib input block
    cannot be opened.

**120: Invalid units in cooling time: <time> <units>**

    The specified cooling time:doc:`[G] <glossarytext.rst>` 
    does not have one of the supported time units.

**121: No after-shutdown/cooling times were defined.**

    The cooling input block contains no information before
    the end keyword:doc:`[G] <glossarytext.rst>`.

**130: Invalid dimension type: <string>**

    The type of dimension:doc:`[G] <glossarytext.rst>`, string,
    declared in the dimension block is not supported.

**131: Dimension has no boundaries**

    The dimension block has no zone boundary information
    before the end keyword.

**140: Invalid flux type: <string>**

    The flux type:doc:`[G] <glossarytext.rst>`, string, specified
    in the flux block in not supported.

**150: Invalid geometry type: <string>**

    The geometry type, string, specified in the geometry block
    is not supported.

**160: History <string> is empty**

    The history input block, string, contains no information
    before the end keyword:doc:`[G] <glossarytext.rst>`.

**170: Material Loading is empty.**

    The mat_loading input block contains no information
    before the end keyword:doc:`[G] <glossarytext.rst>`.

**180: Target materials for reverse calculations can only be 
elements or isotopes and not '<string>'**

    The constituent type, string, given for this target
    material is not supported. It must be either ''element''
    or ''isostope:doc:`[G] <glossarytext.rst>`''.

**181: Invalid material constituent: <string>**

    The constituent type, string, specified for this
    mixture:doc:`[G] <glossarytext.rst>` constituent is not
    supported.

**182: Mixture <string> has no constituents**

    The mixture input block, string, contains no
    information before the end keyword.

**190: Invalid units in pulse level: <time> <units>**

    The specified pulse level decay time does not have
    one of the supported time units.

**200: Schedule <string> is empty**

    The schedule input block, string, contains no
    information before the end keyword.

**210: Invalid units in schedule item delay time: <time> <units>**

    The specified inter-schedule delay time does not
    have one of the supported time units.

**211: Invalid units in single pulse time: <time> <units>**

    The specified pulse length does not have one of
    the supported time units.

**230: Output type '<string>' is not currently supported.**

    The output type, string, specified for this output
    format is not supported.

**240: Unable to open dump file:doc:`[G] <glossarytext.rst>` <string>**

    The output ''dump'' file could not be opened.

Input Checking
==============

**300: Cannot define both zone dimensions and interval volumes.**

    ALARA does not permit the geometry to be defined with
    both the dimension input block and the volumes input
    block. This would result in redundant and possibly
    inconsistent input.

**301: A material loading is given for more zones
(<loaded_zones>) than are defined by the zone dimensions
(<zone_dimensions>). Those extra zones are being ignored.**

    The number of zones as defined by the mat_loading
    input block does is larger than the number defined by
    the dimension blocks. This is permissible, but may lead
    to dubious results. The extra zones from the
    mat_loading block will be ignored.

**302: Material loadings were not defined for as many 
zones (<loaded_zones>) as were defined by the zone 
dimensions (<zone_dimensions>).**

    The number of zones as defined by the mat_loading
    input block is smaller than the number defined by
    the dimension blocks. This is NOT permissible 
    as it would leave some zones unfilled.

**303: Must define either zone dimensions or interval 
volumes for multi-point problems.**

    ALARA requires a definition of the geomery using
    either the dimension input block or the volumes
    input block for problems in more than 0 dimensions.

**310: Could not find element <string> in element library.**

    The element string was not found in the element
    library. This could be due to an error in the
    material library, incorrect user input, or an
    omission in the element library.

**311: Could not find material <string> in material library.**

    The material string was not found in the material
    library. This could be due to incorrect user
    input or an omission in the element library.

**330: Duplicate dimensions of type <string>.**

    The dimension string was defined more that
    once in the input file:doc:`[G] <glossarytext.rst>`.

331: <string1> geometries don't have dimensions of type <string2>.

    The dimension type string2 was defined for
    geometry type string1, which does not allow
    this kind of dimension:doc:`[G] <glossarytext.rst>`.

**340: Unable to open flux file <string1> for flux <string2>.**

    In the flux:doc:`[G] <glossarytext.rst>` definition
    string2 the given flux file string1
    cannot be opened.

**350: Toroidal problems with zone dimensions require a major radius.**

    All problems defined as having toroidal
    geometries:doc:`[G] <glossarytext.rst>` must define
    a major radius:doc:`[G] <glossarytext.rst>` with
    the major_radius input block.

**351: Toroidal problems with zone dimensions require either
a minor radius:doc:`[G] <glossarytext.rst>` or a radius dimension.**

    All problems defined as having toroidal
    geometries:doc:`[G] <glossarytext.rst>` must define
    a minor radius with either a dimension block
    or the minor_radius input block.

**370: Zone <string1> is loaded with a non-existent 
mixture: <string2>**

    The mixture:doc:`[G] <glossarytext.rst>` string2
    specified to fill zone string1 in the mat_loading
    block is not defined in the input file. Either
    add a new mixture definition or change the name
    of the mixture to be used for this zone.

**380: Constituent type 'l' of mixture <string1> references 
a non-existent mixture: <string2>**

    The mixture string2 specified in the ''similar''
    constituent of mixture string1 is not defined
    in the input file. Either add a new mixture
    definition or change the name of the mixture 
    to be used for this definition.

**400: Unable to find top level schedule. A top level 
schedule must not used as a sub-schedule.**

    All of the defined schedules are referenced as
    sub-schedules of other schedules. This means that
    there is no top to the hierarchical schedule
    system, as required.

**410: Flux <string1> for simple pulse item of schedule 
<string2> does not exist.**

    The flux string1 required to calculate the simple
    pulsing schedule item of schedule string2
    is not defined.

**411: Bad flux file for flux <string> for simple pulse
 item of schedule <string>.**

    The file for flux string1 required to calculate
    the simple pulsing schedule item of schedule
    string2 cannot be opened.

**412: Schedule recursion: <string>.**

    There is a loop in the schedule hierarchy. This
    implies an infinitely long and infinitely
    complex total irradiation history, which is
    unphysical. Check the definition of the schedules.

**413: Schedule <string1> for subschedule item of schedule
<string2> does not exist.**

    The sub-schedule string1 defined as a schedule
    item of schedule string2 has not been defined.

**414: Pulse history <string1> for item of schedule 
<string2> does not exist.**

    The pulsing history string1 required to
    calculate a schedule item of schedule
    string2 has not been defined.

**420: Zone:doc:`[G] <glossarytext.rst>` <string> specified in 
interval volumes was not found in the material loading.**

    The zone string specified to contain one
    of the volumes in the volumes input block
    does not exist.

**440: ALARA now requires a binary dump file:doc:`[G] glossarytext.rst>`.
Openning the default file 'alara.dmp'.**

    ALARA uses a binary file to store intermediate
    results. You can set the name of this file
    using the dump_file input block. Otherwise,
    the default is used.

Input Cross-referencing
=======================

**580: Removing mixture <string> not used in any zones.**

    Mixture:doc:`[G] <glossarytext.rst>` string was
    defined in the input file:doc:`[G] <glossarytext.rst>`,
    but is not used in any zones. It's
    definition is being removed.

**620: You have specified too few normalizations. If you 
specifiy any normalizations, you must specify one for 
each interval.**

    The spatial_norm input block must contain
    an entry for each of the fine mesh
    intervals:doc:`[G] <glossarytext.rst>`. It is
    not permissible to have too few.

**621: You have specified too many normalizations. Extra 
normalizations will be ignored.**

    It is permissible to define too many
    spatial normalizations, but the results
    may by dubious. The extra normalizations
    will be ignored.

**622: Flux file <string> does not contain enough data.**

    The flux file:doc:`[G] <glossarytext.rst>` string
    does not contain enough data to provide a
    flux for each of the fine mesh
    intervals:doc:`[G] <glossarytext.rst>`.

Data Library Errors
===================

**1000: Data library type <string> (<type_code>) is not yet supported.**

    The specified library type string is not supported.

**1001: Conversion from <string1> (<type_code>) to <string2> 
(<type_code>) is not yet supported.**

    Conversion between the specified library
    types string1 and string2 is not supported.

**1001: Conversion from <string> (<type_code>) to (<type_code>) 
is not yet supported.**

    Conversion between the specified library
    types string1 and <type_code>
    is not supported.

**1100: You have specified library type 'alaralib' but given 
the filename of an 'adjlib' libra**

    The type of library specified in the input
    block must match the internally recorded
    library type.

**1101: You have specified library type 'alaralib' but given 
the filename of an unidentified library.**

    The type of library specified in the input
    block must match the internally recorded
    library type.

**1102: You have specified library type 'adjlib' but given 
the filename of an 'alaralib' library.**

    The type of library specified in the input
    block must match the internally recorded
    library type.

**1103: You have specified library type 'adjlib' but given 
the filename of an unidentified library.**

    The type of library specified in the input
    block must match the internally recorded
    library type.

Programming Errors
==================

    **Note:**

        In some places, if ALARA reaches that point
        in the program, it implies an error in the
        logic of the code. Please report such
        errors to the code author.

**-1: Memory allocation error: <string>**

    An error in the runtime allocation of memory
    occured. ''<string>'' reports the function
    and variable where the error occurred.

**9000: Programming Error:... **
