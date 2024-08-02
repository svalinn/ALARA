# Import packages
from string import Template
import subprocess
from pathlib import Path
import re

# Define constant(s)
INPUT = 'groupr.inp'

# Create input file general template
njoy_input = Template(Template(
"""groupr/
 $NENDF $NPEND $NGOUT1 $NGOUT2/
 $mat_id $IGN $IGG $IWT $LORD $NTEMP $NSIGZ $IPRINT/
 $title/
 $TEMP
 $SIGZ/
 $reactions
 $MATD/
 0/
stop
"""
).safe_substitute(
    NENDF = 20,                                           # unit for endf tape
    NPEND = 21,                                          # unit for pendf tape
    NGOUT1 = 0,                         # unit for input gout tape (default=0)
    NGOUT2 = 31,                       # unit for output gout tape (default=0)
    IGN = 17,    # neutron group structure option (corresponding to Vitamin J)
    IGG = 0,                                    # gamma group structure option
    IWT = 11,            # weight function option (corresponding to Vitamin E)
    LORD = 0,                                                 # Legendre order
    NTEMP = 1,                            # number of temperatures (default=1)
    NSIGZ = 1,                            # number of sigma zeroes (default=1)
    IPRINT = 1,        # long print option (0/1=minimum/maximum) --(default=1)
    ISMOOTH = 1,        # switch on/off smoother operation (1/0, default=1=on)
    TEMP = 293.16,                                     # temperature in Kelvin
    SIGZ = 0,                         # sigma zero values (including infinity)
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

def fill_input_template(material_id, MTs, element, A, mt_dict): 
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
        MTs (list of int): List of reaction types (MT's) present in the
            ENDF/PENDF files.
        element (str): Chemical symbol for element of interest.
        A (str or int): Mass number for selected isotope.
            If the target is a metastable isomer, "m" is written after the
            mass number, so A must be input as a string.
        mt_dict (dict): Reference dictionary containing reaction information
            for each MT number pre-defined in the ENDF manual.
        template (string.Template): A static string template containing
            the general structure of the NJOY input file to run GROUPR,
            $identifiers for substitutable items, produced by executing
            establish_static_template().
    
    Returns:
        template (str): Modified template with the material-
            specific information substituted in for the $identifiers,
            converted to a string.
    """

    Z = str(elements[element]).zfill(2)
    title = f'"{Z}-{element}-{A} for TENDL 2017"'

    card9_lines = []
    for MT in MTs:
        mtname = mt_dict[MT]['Reaction']
        MFD = 3 # ENDF file tag for cross-section data
        card9_lines.append(f'{MFD} {MT} "{mtname}" /') 
    card9 = '\n '.join(card9_lines)
    return njoy_input.substitute(
        mat_id=material_id,
        title=title,                                            
        reactions=card9,                                        
    )

def write_njoy_input_file(template):
    """
    Write out the NJOY GROUPR input card from the prefilled template.

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
        gendf_path (str): File path to the newly created GENDF file.
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

def run_njoy(element, A, matb):
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
    
    Returns:
        gendf_path (str): File path to the newly created GENDF file. 
    """

    # Run NJOY
    result = subprocess.run(['njoy'], input=open(INPUT).read(),
                        text=True, capture_output=True)

    # If the run is successful, log out the output
    # and make a copy of the file as a .GENDF file
    if not result.stderr:
        gendf_path = f'tendl_2017_{element}{str(A).zfill(3)}.gendf'
        Path('tape31').rename(Path(gendf_path))
        ensure_gendf_markers(gendf_path, matb)

        return gendf_path

def cleanup_njoy_files(output_path = 'njoy_ouput'):
    """
    Clean up repository from unnecessary intermediate files from NJOY run.
    
    Arguments:
        output_path (str, optional): The save path for the NJOY output.
            Defaults to 'njoy_output', which will save the file in the
            same directory as all other saved files from the script run.
    
    Returns:
        None
    """

    njoy_files = [INPUT, 'tape20', 'tape21']
    for file in njoy_files:
        Path.unlink(Path(file))
    Path('output').rename(Path(output_path))