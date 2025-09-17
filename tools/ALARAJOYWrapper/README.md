# FENDL3 Preprocessor with NJOY Wrapping for ALARA (*ALARAJOYWrapper*)

This preprocessor is designed to update ALARA's data input capabilities for updated FENDL3 data formats. Unlike previous versions of [FENDL](https://www-nds.iaea.org/fendl_library/websites/fendl32b/) (Fusion Evaluated Nuclear Data Library) which have available groupwise cross-section data for neutron activation, which is required for ALARA's functionality, FENDL3 data is only offered as pointwise.

This preprocessor uses the [NJOY 2016](https://github.com/njoy/NJOY2016) Nuclear Data Processing System's GROUPR module to convert the pointwise nuclear data to the Vitamin-J 175 group groupwise format for cross-sections and handle the processed data to be fed back to ALARA.

## Dependencies

* [ENDFtk](https://github.com/njoy/ENDFtk)
* [NJOY 2016](https://github.com/njoy/NJOY2016)
* [Pandas](https://pandas.pydata.org/docs/getting_started/install.html)
* [NumPy](https://numpy.org/install/)
* [ArgParse](https://docs.python.org/3/library/argparse.html)
* [String](https://docs.python.org/3/library/string.html#module-string)
* [Subprocess](https://docs.python.org/3/library/subprocess.html)
* [Pathlib](https://docs.python.org/3/library/pathlib.html)
* [Re](https://docs.python.org/3/library/re.html)
* [Os](https://docs.python.org/3/library/os.html)
* [Csv](https://docs.python.org/3/library/csv.html)
* [Pytest](https://docs.pytest.org/en/stable/getting-started.html)

## Usage
This preprocessor can and should be used independently of any ALARA run. While the data format produced from ALARAJOYWrapper is necessary to run `convert_lib` on ALARA to convert FENDL3 data to an ALARA binary, once the user has a CSV file produced from this preprocessor, usage of this preprocessor is not strictly necessary for the particular isotopes processed by the user.

ALARAJOYWrapper is designed to produce a CSV containing data that can be directly read by the ALARA data library, ALARAJOY, without any further processing.

To run this preprocessor, the user must first have acquired matching TENDL/PENDF file pairs for each isotope to be processed from [TENDL 2017](https://tendl.web.psi.ch/tendl_2017/tendl2017.html), which is the source for FENDL3 neutron activation data. All TENDL/PENDF file pairs to be processed must be in the same directory as each other to be properly identified by ALARAJOYWrapper.

Running ALARAJOYWrapper can be done with one Python command:

    python preprocess_fendl3.py -f /path/to/fendl3_data_dir/

This command only takes one argument, `-f`, which directs the program to the directory containing the TENDL/PENDF file pairs. This argument is optional, and if left blank, will default to the current working directory.

## Data Output
Running `preprocess_fendl3.py` will return the file path to the resultant CSV file containing transmutation reaction pathways for the neutron activation of the given isotope(s). Each row in the CSV represents a different reaction, and contains the following data needed by ALARA for a library conversion:

- Parent KZA: Unique isotope identifier for the parent isotope in the format **ZZAAAM**, where ZZ is the isotope's atomic number, AAA is the mass number, and M is the isomeric state (0 if non-excited).
- Daughter KZA: Unique isotope identifier of the daughter isotope produced from a particular transmutation reaction in the format **ZZAAAM**.
- Emitted Particles: Consecutive string of particles emitted from the transmutation reaction. The possible particles are:
  - Neutron (n)
  - Proton (p)
  - Deuteron (d)
  - Triton (t)
  - <sup>3</sup>He nucleus (h)
  - Alpha particle (a)
  - Gamma photon (g)


  For reactions with multiple emitted particles, the format of the text string is along the lines '3n2p', representing an emission of three neutrons and two protons.

- Non-Zero Groups: Count of total number of energy groups in the Vitamin-J 175 groups structure with non-zero neutron cross sections.
- Cross Sections: List of all non-zero neutron cross sections.

## Application of Processed Data to ALARA Data Conversion Methods
Data library conversion to ALARA binary libraries is done with the `convert_lib` input block in the ALARA input file. Converting preprocessed FENDL3 data contained in the resultant CSV from ALARAJOYWrapper is done as such in the input file:

    convert_lib ajoylib alaralib transFname decayFname alaraFname
wherein: 
- `ajoylib` is the library value for ALARAJOY
- `transFname` is the file path to the CSV containing the preprocessed FENDL3 transmutation data
- `decayFname` is the file path to EAF decay data.
- `alaraFname` is the file path template for resultant ALARA data libraries (.lib, .idx. .gam, .gdx)

The ALARA run for library conversion should precede and be independent from subsequent activation calculations with ALARA, as specified in the [ALARA Users' Guide](https://svalinn.github.io/ALARA/usersguide/index.html):

> *This input block is used to convert library formats. If this input block is included, ALARA will stop immediately after converting the library (ie. it should not be used as part of a normal ALARA input file).*