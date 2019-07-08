==================
ALARA Users Guide:
==================

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
 useful :doc:`error messages <errortext.rst>` when the data
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
 file:doc:`[G] <glossarytext.rst>`:

 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+
 |                          |                           |                          |
 +--------------------------+---------------------------+--------------------------+


General Input Notes:
====================

 1. **Dimension/Volume:** If “[1]” follows an input
    block name, either the dimension:doc:`[G] <glossarytext.rst>`
    or the volume is required, but it will generate an
    error if both are included. 

 2. **Input Blocks:** Not all input blocks are required,
    with some being unnecessary for certain problems. Input
    blocks indicated in bold are required at least once in
    every input file:doc:`[G] <glossarytext.rst>`.

    There are also some input blocks which are incompatible
    with each other. While superfluous input blocks may go
    unnoticed (there are occasional warnings), incompatible
    input blocks will create an error.

 3. **Naming Input Data:** Most input blocks allow the user
    to define their own symbolic names for cross-referencing
    the various input data. Any string of characters can be
    used as long as its does not contain any
    whitespace:doc:`[G] <glossarytext.rst>` (spaces, tabs,
    new-lines, etc.).

    It is considered dangerous, however, to use a
    keyword:doc:`[G] <glossarytext.rst>` as a symbolic name.
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

    [s]econd
    [m]inute             60 seconds
    [h]our               60 minutes
    [d]ay                24 hours
    [w]eek               7 days
    [y]ear               52 weeks
    [c]entury            100 years


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
    value with no whitespace:doc:`[G] <glossarytext.rst>`)
    by using the same comment character (#). Such
    comments extend to the end of the current line.
    Blank lines are permitted anywhere in the input
    file:doc:`[G] <glossarytext.rst>`. 

 8. **Length Units:** Centimeters should be used for all
    length units.

