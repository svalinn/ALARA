==================
ALARA Users' Guide
==================

Manual Syntax
=============

 :doc:`[G] <glossarytext.rst>`             This icon indicates that a definition exists in the
					   glossary for this term.
 :doc:`command line <commandtext.rst>`     An underlined term indicates that there is a
					   section in the manual on this term.
 dimension                                 Text that is written in courier new indicates actual
					   code syntax.
 <OPTION>                                  The term option in brackets indicates a choice in
					   syntax. These choices are given in the option
					   description section of the corresponding file description. 


Time Based Input
================

 Some input elements represent times and can be defined in a number of
 different units. When this is the case, the floating point time value
 should be followed by a single character representing the following units: 

   [s]econd
   [m]inute	60 seconds
   [h]our	60 minutes
   [d]ay	24 hours
   [w]eek	7 days
   [y]ear      	52 weeks
   [c]entury 	100 years

     Sample inputs - the following are equivalent: 

       * 10 d
       * 240 h
       * 14400

Length Units
============

 Centimeters should be used for all length units. 
