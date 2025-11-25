# Sample ALARA Files

These sample files have been developed primarily to support regression testing
for ALARA.  As such, they may not be valuable files for demonstrating how to use
ALARA, but do attempt to cover a range of input file complexity.

All the samples use data files made available in the `data` subdirectory. More
details are given below.

A set of reference outputs is provided (`output_ref`) to test that ALARA is
producing consistent results.

## sample1

This sample file exists only to generate an ALARAlib data library for use in the
other tests.  This test uses a truncated version of the FENDL2.0 data library
prepared in a multigroup format consistent with ALARA's EAF library format.
It generates an ALARA-formatted data library called `truncated_fendl2bin`.

## sample2

This is the simplest sample file, with:
* rectangular geometry
* consisting of two zones
* each with a single interval
* each of which has a mixture of materials
* with those materials defined in the provided material library

The neutron flux is read from the `fluxin1` file and not scaled.

There is a single 1 year long pulse.

Results are produced at 4 cooling times for the number density and specific
activity.

## sample3

This relatively simple sample file has:
* cylindrical geometry
* consisting of two zones
* containing 1 and 2 intervals, respectively
* each of which has a mixture of materials and elements
* with those materials and elements defined in the provided material and element
  libraries

The flux is read from the `fluxin2` file and scaled by 1e6.

The pulsing history has a 1 second pulse, repeated 10 times with 5 seconds
between each pulse.

Results are produced at 4 cooling times for the total_heat, separated by the
constituents of each material.

## sample4

This sample file adds complexity, and includes:
* cylindrical geometry
* consisting of three zones
* each containing a single interval
* each of which has a mixture of materials
* including one marked as `void`
* with those materials and elements defined in the provided material and element
  libraries

The flux is read from the `hohlflux`, skipping the first 90 entries and scaling the values by 1e-10.

The irradiation schedule is more complex, with 
* a 1 day pulse repeated 25 times with 25 hours between pulses, followed by
* a 25 hour delay, followed by
* a 5 minute pulse repeated once

Results are produced at 5 cooling times for the waste 
disposal rating for NRC Class A and Class C waste, which
require the activity to be in units of Ci/m3.

## sample5

This advanced sample includes:
* a geometry with three intervals with different volumes
* each in a separate zone
* each of which has a mixture,
   * one with an element, 
   * one with an enriched element, and 
   * one with a material
* with those materials and elements defined in the provided material and element
  libraries

The flux is read from the `fluxin1`, and normalized differently for each interval.

There is a single 10 year pulse.

The total heat and photon source, with units of Bq per cm3, are produced as results at 11 cooling times.

Most initial nuclides will have their chains truncated with a tolerance of 1e-6,
but impurities (defined as nuclides with an initial concetration of less than 10 ppm) will be truncated
with a tolerance of 1e-2.

## sample6

This sample file exists only to generate an adjoint ALARAlib data library for use in the
other tests.  This test uses a truncated version of the FENDL2.0 data library
that has already been converted to an ALARALib in sample1.
It generates an ALARA-formatted adjoint data library called `truncated_fendl2adj`.

## sample7

This sample file combines a number of expert features:
* a 2-D toroidal geometry with major radius 30 cm
* with 1 zone in each dimension
* and one interval per zone
* resulting in a single zone
* containing a single material

Three different flux spectra read read from `fluxin1`, `fluxin2`, and `fluxin3`, none of which are scaled. 

A multi-level pulsing schedule consists of
* a 10 s pulse of flux 1, followed by
* a 12 second delay, followed by
* a 10 s pulse of flux 2, followed by
* a 3 s delay, followed by
* a 10 s pulse of flux 3, followed by
* a 5 s delay, followed by
* 3 repetitions of the following schedule each with 100 s between them
    * a 5 s pulse of flux 2 repeated twice, with a 10 s
      between each pulse

This input file demonstrates "reverse" mode by using an adjoing ALARA-library,
and therefore requires that at least one target nuclide be identified in a
material, in this case isotopes of the element He.

Results are given in number density for the zone in 
units of Bq/cm3.

## sample8

This file uses simple geometry, mixtures and schedule to
focus on different ways to estimate the biological dose.

* rectangular geometry
* with 2 zones
* with 3 and 1 intervals, respectively
* each zone has a different mixture, including one as `void`
* the mixture defined with elements from the element library

The flux is read from `fluxin` and not scaled, but the first entry is skipped

A single 5 year pulse occurs.

The output is given for 4 cooling times, including
both the contact dose using ANS standard 6.4.3, and 
a dose estimate that comes from folding the photon source
with an adjoint photon dose response, with a detector of
volume 0.5 cm3 and the adjoint response defined in `adjfile2.gam` using 21 groups as defined here.

## sample9

This example is used to demonstrate the (undocumented) `integrate_energy` option
for generating photon source output.  This options changes how dicrete
line emissions are integrated into multigroup photon sources.

# Sample Data

The data directory contains a variety of files for use in these sample problems:
* Flux files have 175 group flux spectra with at least enough for each interval
  in the problem: `fluxin1`, `fluxin2`, `fluxin3`, `hohlflux`, `fluxin_zeros`
* A sample material library (`sampleMatlib`) describes how to build a material 
  out of elements
* A standard element library (`myElelib`) describes the isotopic composition of
  standard elements, and demonstrates how to define enriched elements
* a truncated version of the FENDL2.0 multi-group neutron cross sections and
  decay data including gamma decay details
* an adjoint flux (`adjfil2.gam`) used to demonstrate the ability to fold a
  photon source with the adjoint photon flux to estimate detector response
* data files related to output including waste disposal ratings (`NRCA`, `NRCC`)
  and contact dose (`ANS6_4_3`)
