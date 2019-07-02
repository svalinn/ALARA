===================
ALARA Users' Guide:
===================

Analytic and Laplacian Adaptive Radioactivity Analysis
======================================================

--------------

INTRODUCTION:
=============

 The primary purpose of ALARA is to calculate the induced
 activation :doc:`[G] <glossarytext.rst>` caused by neutron
 irradiation :doc:`[G] <glossarytext.rst>` throughout a nuclear
 system (including fission reactors, fusion reactors,
 and accelerators). The usage of ALARA is fairly straightforward,
 requiring little knowledge of the code's inner workings.
 This Users' Guide, however does assume that the reader is
 familiar with the basic problem of induced activation
 :doc:`[G] <glossarytext.rst>`. Background on induced activation, and
 details on ALARA's can be found in [ref. 1]. This reference
 will also help ensure that ALARA is well-suited to the
 problems that you are trying to solve.

 This Users' Guide will describe the command-line options of
 ALARA, input file :doc:`[G] <glossarytext.rst>` structure
 and the basic support files necessary to run ALARA.

FEATURES:
=========

 ALARA takes full advantage of being a newly developed code
 by implementing the best features of the previous
 generation of activation codes and, at the same time,
 addressing their various weaknesses. Furthermore, an entirely
 new set of features has been implemented and continued
 development will allow it to respond to the evolving needs
 of the entire user community. ALARA's three main design
 principles are accuracy, speed, and usability. ALARA matches
 the previous generation of activation codes in accuracy
 and is much faster. This is achieved by adaptively
 choosing the mathematical method at the smallest resolution
 in order to optimize both speed and accuracy.

 The following features are found individually in other
 activation codes (primarily FISPACT, DKR and RACC): 
  
   * multi-point (3-D) solutions in a variety of geometries
   * accurate solution of loops in activation trees
   * exact modeling of multi-level pulsing irradiation
     histories :doc:`[G] <glossarytext.rst>`
   * user-defined calculation precision/accuracy
   * tracking the accumulation of light ions     

 The following features are not found in other activation
 codes, but should be considered basic elements of any
 modern activation code: 

   * straightforward, user-friendly input file creation
   * full, easy-to-read activation tree output (not just
     pathway analysis)
   * flexible output options NOW including the direct
     calculation of waste disposal rating :doc:`[G] <glossarytext.rst>`
     and clearance indices :doc:`[G] <glossarytext.rst>`. 

 The following ADVANCED features are unique to ALARA
 and its development history: 

   * unlimited number of reaction channels
   * exact modeling of hierarchical arbitrary irradiation schedules
   * reverse calculation mode 

 Finally, ALARA is undergoing continuous development with
 many new features planned for implementation, both to
 increase the level of physical modeling, the range of
 problems which can be solved, and its ease of use. ALARA
 is a fully developed and validated alternative to the
 previous generation of activation codes. 
