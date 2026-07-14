# FENDL3.2x Preprocessor with NJOY Wrapping for ALARA (*ALARAJOYWrapper*)

This preprocessor is designed to update ALARA's data input capabilities for updated FENDL3.2x data sets formatted as TENDL (ENDF-6) files. Unlike previous versions of [FENDL](https://www-nds.iaea.org/fendl/) (Fusion Evaluated Nuclear Data Library), which have available groupwise cross-section data for neutron activation, as is required for ALARA's functionality, FENDL3.2x data requires conversion to that format.

This preprocessor uses [NJOY 2016](https://github.com/njoy/NJOY2016) Nuclear Data Processing System to produce requisite pointwise data (PENDF) to subsequntly convert to the Vitamin-J 175 energy group groupwise format (GENDF) for activation cross-sections and to handle the processed data to be fed back to ALARA. For pointwise conversion, ALARAJOYWrapper uses the NJOY modules MODER, RECONR, BROADR, UNRESR, and GASPR, and for the ultimate conversion to the groupwise format, GROUPR.

## Dependencies

- Standard Python libraries
  * [ArgParse](https://docs.python.org/3/library/argparse.html)
  * [Collections](https://docs.python.org/3/library/collections.html)
  * [Csv](https://docs.python.org/3/library/csv.html)
  * [Pathlib](https://docs.python.org/3/library/pathlib.html)
  * [Pytest](https://docs.pytest.org/en/stable/getting-started.html)
  * [Re](https://docs.python.org/3/library/re.html)
  * [String](https://docs.python.org/3/library/string.html#module-string)
  * [Subprocess](https://docs.python.org/3/library/subprocess.html)
  * [Warnings](https://docs.python.org/3/library/warnings.html)
- Generic Python packages
  * [Matplotlib.pyplot](https://matplotlib.org/3.5.3/api/_as_gen/matplotlib.pyplot.html)
  * [NumPy](https://numpy.org/install/)
  * [Pandas](https://pandas.pydata.org/docs/getting_started/install.html)
- Domain-specific packages
  * [ENDFtk](https://github.com/njoy/ENDFtk)
  * [NJOY 2016](https://github.com/njoy/NJOY2016)
  * [OpenMC](https://docs.openmc.org/en/stable/quickinstall.html) (only needed if specifying a multigroup energy structure by name from the dictionary `openmc.mgxs.GROUP_STRUCTURES`)



## Data Acquisition

Activation/inventory solvers such as ALARA require both neutron activation cross-sections and decay data. ALARAJOYWrapper is designed with the specific intent for processing FENDL3.2x data in a format readable by ALARA, corresponding to TENDL-2017 activation cross-sections and UKDD-2020 decay data. Nevertheless, to create custom ALARA libraries, any combination of TENDL and EAF or UKDD data can be supplied to ALARAJOYWrapper.

Many decay data libraries are distributed in repositories containing individual data files for each nuclide in the library. ALARA requires a single decay data file, so ALARAJOYWrapper can also compile these data libraries into a single file referable by `decayFname` (see _Application of Processed Data to ALARA Data Conversion Methods_ below). 

- Activation Cross-Sections
  * [TENDL](https://tendl.imperial.ac.uk/)
     - Usable with any TENDL release
     - For FENDL3.2x processing, [TENDL 2017](https://tendl.imperial.ac.uk/tendl_2017/tendl2017.html) is the standard version.

- Decay Data
  * EAF
      - [EAF-2010](https://git.oecd-nea.org/fispact/nuclear_data/EAF2010data.tar.bz2) (Must separate out ` eaf_dec_20100.0*` from the rest of the files after unzipping)
      - [EAF-4.1](https://nds.iaea.org/fendl20/fen-decay.htm) (FENDL2.x decay library)
  * UKDD
      - [UKDD-20](https://www.oecd-nea.org/dbdata/fispact/decay2020.tar.bz2) (FENDL3.2x decay library)
      - [UKDD-12](https://git.oecd-nea.org/fispact/nuclear_data/decay.tar.bz2)


## Usage
This preprocessor can and should be used independently of any ALARA run. While the data format produced from ALARAJOYWrapper is necessary to run `convert_lib` on ALARA to convert TENDL data to an ALARA binary, once the user has a space-delimited DSV file produced from this preprocessor, usage of this script is not strictly necessary for the particular isotopes processed by the user.

ALARAJOYWrapper is designed to produce a space-delimited DSV containing cross-data that can be directly read by the ALARA data library, ALARAJOY, without any further processing.

Running ALARAJOYWrapper can be done with one Python command:
```
python preprocess_fendl3.py -f /path/to/fendl3_data_dir/ -d /path/to/decay_library/ decay_library-type -g group_name -a -t -r -p
```
To read in detail about each of these arguments, call this command:
```
python preprocess_fendl3.py -h
```


## Data Output
Running `preprocess_fendl3.py` will produce two file paths. The first is to the compiled decay data file, which will either be identical to the input for the first argument in `-d` if a pre-compiled decay library is being used or to the newly produced compiled decay library. The second is to the resultant space-delimited DSV file containing transmutation reaction pathways for the neutron activation of the given isotope(s). The header of this DSV file will contain two entries, the number of groups of the group structure according to which the data was converted and the name of said group-structure. Each row in the DSV represents a different reaction, and contains the following data needed by ALARA for a library conversion:

- Parent KZA: Unique isotope identifier for the parent isotope in the format **ZZAAAM**, where ZZ is the isotope's atomic number, AAA is the mass number, and M is the isomeric state (0 if non-excited).
- Daughter KZA: Unique isotope identifier of the daughter isotope produced from a particular transmutation reaction in the format **ZZAAAM**.
- Emitted Particles: Consecutive string of particles emitted from the transmutation reaction. The possible particles are:
  - Neutron (n)
  - Gamma photon (g)
  - Proton (p)
  - Deuteron (d)
  - Triton (t)
  - <sup>3</sup>He nucleus (h)
  - Alpha particle (a)

  For reactions with multiple emitted particles, the format of the text string is along the lines '3n2p', representing an emission of three neutrons and two protons. An emission containing only one of a certain particle will just contain the particle identifier, without any numerical tag (e.g. 'n' for a single neutron).

- Cross Sections: List of all non-zero neutron cross sections.

It should be noted that TENDL 2017 activation data contains many reactions that leave the residual nucleus in various quantized excited states, potentially up to a 40<sup>th</sup> excited state. Most of these daughter isomers, however, are extremely short-lived and lack decay data. When converting an ALARAJOY DSV file in ALARA, as discussed below, an EAF decay library is required, given that FENDL3.2x only contains activation data. Cross-referencing exotic isomers against this decay data ensures that ultra-short-lived reaction products are not passed on to ALARA's library conversion appearing as stable nuclides. Rather, the lack of decay data signifies such a short half-life that decay evaluations are not practical. For excited daughter nuclides produced from a given reaction lacking corresponding decay data, ALARAJOY will incrementally de-excite the nuclear state one-by-one down to the next lowest energy level with known decay data (or ultimately to ground).

## Application of Processed Data to ALARA Data Conversion Methods
Data library conversion to ALARA binary libraries is done with the `convert_lib` input block in the ALARA input file. Converting preprocessed TENDL data contained in the resultant space-delimited DSV from ALARAJOYWrapper is done as such in the input file:

    convert_lib ajoylib alaralib transFname decayFname alaraFname
wherein: 
- `ajoylib` is the library value for ALARAJOY
- `transFname` is the file path to the DSV containing the preprocessed FENDL3 transmutation data
- `decayFname` is the file path to the compiled EAF or UKDD decay library, which may be produced by ALARAJOYWrapper.
- `alaraFname` is the file path template for resultant ALARA data libraries (.lib, .idx. .gam, .gdx)

The ALARA run for library conversion should precede and be independent from subsequent activation calculations with ALARA, as specified in the [ALARA Users' Guide](https://svalinn.github.io/ALARA/usersguide/index.html):

> *This input block is used to convert library formats. If this input block is included, ALARA will stop immediately after converting the library (ie. it should not be used as part of a normal ALARA input file).*