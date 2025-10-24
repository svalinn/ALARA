# Import packages
from string import Template
import subprocess
from pathlib import Path
import re
from shutil import copy

def set_directory():
    '''
    Establish the location of the current working directory to ensure that if
        process_fendl3.2.py is called from ALARA/src/DataLib, FENDL3.2b
        preprocessing files can be properly located, created, and modified.
    Arguments:
        None
    Returns:
        dir (pathlib._local.PosixPath): Path to the current working directory
            (CWD) from which the command was called.
    '''

    return Path(__file__).resolve().parent

def set_directory():
    '''
    Establish the location of the current working directory to ensure that if
        process_fendl3.2.py is called from ALARA/src/DataLib, FENDL3.2b
        preprocessing files can be properly located, created, and modified.
    Arguments:
        None
    Returns:
        dir (pathlib._local.PosixPath): Path to the current working directory
            (CWD) from which the command was called.
    '''

    return Path(__file__).resolve().parent

# Define constant(s)
dir = set_directory()
INPUT = dir / 'njoy.inp'

# Create input file general template
njoy_prep_input = Template(Template(
"""
moder
 $NIN_moder $NOUT_moder/
reconr
 $NENDF $NPEND_reconr/
 'neutron PENDF for $element-$a of TENDL-2017'
 $mat_id/
 $ERR/
 $MATD/
broadr
 $NENDF $NPEND_reconr $NPEND_broadr/
 $mat_id $N_FINAL_TEMPS/
 $ERRTHN/
 $final_broadr_temp/
 $MATD/
unresr
 $NENDF $NIN_unresr $NOUT_unresr/
 $mat_id $NTEMP $NSIGZ $IPRINT_unresr/
 $self_shielding_temp/
 $SIGZ_unresr/
 $MATD/
gaspr
 $NENDF $NPEND_gaspr $NOUT_gaspr/
stop
"""
).safe_substitute(
    NIN_moder = 20,                                         # MODER input unit
    NOUT_moder = 21,                                       # MODER output unit
    NENDF = 21,                # unit for endf tape (equivalent to NOUT_moder)
    NPEND_reconr = 22,                   # unit for RECONR-produced pendf tape
    ERR = 0.001,                         # fractional reconstruction tolerance
    NPEND_broadr = 23,                   # unit for BROADR-produced pendf tape
    N_FINAL_TEMPS = 1,            # number of final temperatures (default = 1)
    ERRTHN = 0.001,                        # fractional tolerance for thinning
    NIN_unresr = 23,  # unit for input pendf tape (equivalent to NPEND_broadr)
    NOUT_unresr = 24,                    # unit for UNRESR-produced pendf tape
    NTEMP = 1,                            # number of temperatures (default=1)
    NSIGZ = 1,                            # number of sigma zeroes (default=1)
    IPRINT_unresr = 0,         # print option for UNRESR (0/1=minimum/maximum)
    SIGZ_unresr = 1.e10,   # sigma zero values (including infinity) for UNRESR
    NPEND_gaspr = 24,  # unit for input pendf tape (equivalent to NOUT_unresr)
    NOUT_gaspr = 25,                      # unit for GASPR-produced pendf tape
    MATD = 0,                                # next mat number to be processed
))

groupr_input = Template(Template(
"""
groupr/
 $NENDF $NPEND $NGOUT1 $NGOUT2/
 $mat_id $IGN $IGG $IWT $LORD $NTEMP $NSIGZ $IPRINT_groupr/
 $title/
 $groupr_temp
 $SIGZ_groupr/
 $reactions
 $MATD/
 0/
stop
"""
).safe_substitute(
    NENDF = 21,                # unit for endf tape (equivalent to NOUT_moder)
    NPEND = 25,         # unit for final PENDF tape (equivalent to NOUT_gaspr)
    NGOUT1 = 0,                         # unit for input gout tape (default=0)
    NGOUT2 = 31,                       # unit for output gout tape (default=0)
    IGN = 17,    # neutron group structure option (corresponding to Vitamin J)
    IGG = 0,                                    # gamma group structure option
    IWT = 11,            # weight function option (corresponding to Vitamin E)
    LORD = 0,                                                 # Legendre order
    NTEMP = 1,                            # number of temperatures (default=1)
    NSIGZ = 1,                            # number of sigma zeroes (default=1)
    IPRINT_groupr = 1, # long print option (0/1=minimum/maximum) - (default=1)
    ISMOOTH = 1,        # switch on/off smoother operation (1/0, default=1=on)
    SIGZ_groupr = 0,       # sigma zero values (including infinity) for GROUPR
    MATD = 0,                                # next mat number to be processed
))

# Define a dictionary of elements in the Periodic Table
elements = [
    'H', 'He', 'Li', 'Be', 'B', 'C', 'N', 'O', 'F', 'Ne',
    'Na', 'Mg', 'Al', 'Si', 'P', 'S', 'Cl', 'Ar', 'K', 'Ca',
    'Sc', 'Ti', 'V', 'Cr', 'Mn', 'Fe', 'Co', 'Ni', 'Cu', 'Zn',
    'Ga', 'Ge', 'As', 'Se', 'Br', 'Kr', 'Rb', 'Sr', 'Y', 'Zr',
    'Nb', 'Mo', 'Tc', 'Ru', 'Rh', 'Pd', 'Ag', 'Cd', 'In', 'Sn',
    'Sb', 'Te', 'I', 'Xe', 'Cs', 'Ba', 'La', 'Ce', 'Pr', 'Nd',
    'Pm', 'Sm', 'Eu', 'Gd', 'Tb', 'Dy', 'Ho', 'Er', 'Tm', 'Yb',
    'Lu', 'Hf', 'Ta', 'W', 'Re', 'Os', 'Ir', 'Pt', 'Au', 'Hg',
    'Tl', 'Pb', 'Bi', 'Po', 'At', 'Rn', 'Fr', 'Ra', 'Ac', 'Th',
    'Pa', 'U', 'Np', 'Pu', 'Am', 'Cm', 'Bk', 'Cf', 'Es', 'Fm',
    'Md', 'No', 'Lr', 'Rf', 'Db', 'Sg', 'Bh', 'Hs', 'Mt', 'Ds',
    'Rg', 'Cn', 'Nh', 'Fl', 'Mc', 'Lv', 'Ts', 'Og'
]
elements = dict(zip(elements, range(1, len(elements)+1)))

def fill_input_template( material_id, MTs, element, A, mt_dict, temperature, run_type=None):
    """
    Substitute in the material-specific values for a given ENDF/PENDF file
        into the template for the NJOY input card. These values are the
        material ID, the title, which incorporates the isotopic description,
        and the reactions corresponding to the MT numbers encoded in the
        files.
    Arguments:
        material_id (int): Unique material identifier, defined by the ENDF-6
            Formats Manual
            (https://www.oecd-nea.org/dbdata/data/manual-endf/endf102.pdf).
        MTs (list or set): List or set of reaction types (MT's) present in the
            ENDF/PENDF files.
        element (str): Chemical symbol for element of interest.
        A (str or int): Mass number for selected isotope.
            If the target is a metastable isomer, "m" is written after the
            mass number, so A must be input as a string.
        mt_dict (dict): Reference dictionary containing reaction information
            for each MT number pre-defined in the ENDF manual.
        temperature (float): Temperature at which to run NJOY modules.
        run_type (str or None): Specification for type of NJOY run to be
            prepared (i.e. preparing and creating PENDFs or converting to a
            group-structured GENDF).
            (Defaults to None)
    
    Returns:
        template (str): Modified template with the material-
            specific information substituted in for the $identifiers,
            converted to a string.
    """
    
    inp = groupr_input if run_type == 'GROUPR' else njoy_prep_input

    Z = str(elements[element]).zfill(2)
    title = f'"{Z}-{element}-{A} for TENDL 2017"'

    card9_lines = []
    MFD = 3 # ENDF file tag for cross-section data
    for MT in MTs:
        mtname = mt_dict[MT]['Reaction']
        card9_lines.append(f'{MFD} {MT} "{mtname}" /') 
    card9 = '\n '.join(card9_lines)
    return inp.substitute(
        element=element,
        a=A,
        mat_id=material_id,
        self_shielding_temp=temperature,
        final_broadr_temp=temperature,
        groupr_temp=temperature,
        title=title,                                            
        reactions=card9,                                        
    )

def write_njoy_input_file(template):
    """
    Write out the NJOY input card from the prefilled template.
    Arguments:
        template (str): A filled template string containing all of the
            parameters to be written out to the NJOY input card.
    Returns:
        None
    """

    with open(INPUT, 'w') as f:
        f.write(template)

def insert_after_identifier(file_str, identifier, record_for_insertion):
    """
    Insert a new line immediately after the last occurrence of a line
        containing the specified identifier in the file string.
    Arguments:
        file_str (str): The content of the GENDF file as a string.
        identifier (str): The identifier to search for in the file content.
        record_for_insertion (str): The content to insert after the identified
            line.
    Returns:
        file_str (str): The (potentially) updated content of the GENDF file.
    """

    pattern = f'({re.escape(identifier)}.*?\n)'
    matches = list(re.finditer(pattern, file_str))
    if matches:
        insert_position = matches[-1].end()
        file_str = (
            file_str[:insert_position] + record_for_insertion
            + file_str[insert_position:]
        )

    return file_str

def ensure_gendf_markers(gendf_path, matb):
    """
    Edit the GENDF files produced from an NJOY GROUPR run to include file and
        section records that are not automatically written out to the file by
        NJOY. Missing these records will not cause errors, but will raise
        messages when they are read by ENDFtk, which expects these markers, so
        this method ensures that they are present. Edits will only be made if
        the SEND record for MF1 or the FEND record for MF3 are not present.
        
        In ENDF-6 formatting, the SEND record signifies the end of a section
        and the FEND record signifies the end of a "file". Note that the use
        of the term "file" here refers to ENDF-6 specific terminology
        referring to a hierarchy of files within a "tape", which in more
        common terminology would be the ENDF, PENDF, or GENDF file itself.
        
        If missing, the MF1 SEND record will be inserted at the last line of 
        the first section of the first "file" within the GENDF "tape".
        Separately, if missing, the MF 3 FEND record will be inserted at the
        end of the third "file", which will be three lines before the end of
        the tape itself.
        The formatting for these records can be found at:
        https://t2.lanl.gov/nis/endf/intro06.html
    Arguments:
        gendf_path (pathlib._local.PosixPath): File path to the newly created
            GENDF file.
        matb (int): Unique material ID for the material in the GENDF file.
    
    Returns:
        None
    """

    # In ENDF-6 formatted files, there are 66 characters/columns of whitespace
    # before the values in record-keeping lines
    whitespace = ' ' * 66

    # Define identifiers and corresponding new lines
    mf1_identifier = f'{matb} 1451   '
    mf3_identifier = f'{matb} 3  099999'
    matb = str(matb).zfill(4)
    mf1_SEND_RECORD = f'{whitespace}{matb} 1  099999 \n'
    mf3_FEND_RECORD = f'{whitespace}{matb} 0  0    0 \n'

    updates = [
        (mf3_identifier, mf3_FEND_RECORD),
        (mf1_identifier, mf1_SEND_RECORD)
    ]

    with open(gendf_path, 'r') as gendf_file:
        file_str = gendf_file.read()

    for identifier, record_for_insertion in updates:
        file_str = insert_after_identifier(file_str, identifier,
                                           record_for_insertion)

    with open(gendf_path, 'w') as gendf_file:
        gendf_file.write(file_str)

def run_njoy(element, A, matb, file_capture):
    """
    Use subprocess to run NJOY given a pre-written input card to convert a
        pair of ENDF and PENDF files to a GENDF file and save it locally.
        The ENDF and PENDF files must be stored in the same directory and be
        titled 'tape20' and 'tape21' respectively for a successful NJOY run.
    
    Arguments:
        cards (dict): Dictionary containing each "card"
            identified by its card number.
        element (str): Chemical symbol for element of interest.
        A (str or int): Mass number for selected isotope.
            If the target is an isomer, "m" after the mass number,
            so A must be input as a string.
        matb (int): Unique material ID for the material in the files.
        file_capture (str): Type of file to be saved from this particular 
            iteration of NJOY runs. Either "PENDF" or "GENDF".
    
    Returns:
        file_metadata['GENDF']['save'] (pathlib._local.PosixPath or None):
                                    File path to the newly created GENDF file.
                                    Returns None if NJOY runs unsuccessfuly.
        result.stderr (str or None): Output of NJOY error.
                                    Returns None if NJOY runs successfully. 
    """

    file_metadata = {
        'GENDF' : { 'dir' : 'gendf_files', 'ext' : '.gendf', 'tape' : 31 },
        'PENDF' : { 'dir' : 'pendf_files', 'ext' : '.pendf', 'tape' : 25 }
    }

    # Run NJOY
    result = subprocess.run(['njoy'], input=open(INPUT).read(),
                        text=True, capture_output=True)

    # If the run is successful, log out the output
    # and make a copy of the file as a .GENDF file
    if not result.stderr:
        for fileinfo in file_metadata.values():
            save_path = dir / fileinfo['dir'] / f'tendl_2017_{element}{str(A).zfill(3)}'

            # Ensure existence of save directory for PENDF/GENDF files
            (dir / fileinfo['dir']).mkdir(exist_ok=True)

            fileinfo['save'] = save_path.with_suffix(fileinfo['ext'])
            Path(f'tape{fileinfo['tape']}').rename(fileinfo['save'])
            if fileinfo['ext'] == '.gendf':
                ensure_gendf_markers(fileinfo['save'], matb)
            
    return fileinfo['save'], result.stderr

def cleanup_njoy_files(output_path = dir / 'njoy_ouput'):
    """
    Clean up repository from unnecessary intermediate files from NJOY run.
    
    Arguments:
        output_path (pathlib._local.PosixPath, optional): The save path for
            the NJOY output.
            Defaults to f'{CWD}/njoy_output', which will save the file in the
            same directory as all other saved files from the script run.
    
    Returns:
        None
    """

    intermediate_files = [INPUT] + [Path(f'tape{i}') for i in range(20,25)]
    for file in intermediate_files:
        Path.unlink(file)
    Path('output').rename(output_path)