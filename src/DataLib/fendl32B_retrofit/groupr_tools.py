# Import packages
import subprocess
from logging_config import logger
from pathlib import Path
from reaction_data import establish_directory

# Define constants
NENDF = 20 # unit for endf tape
NPEND = 21 # unit for pendf tape
NGOUT1 = 0 # unit for input gout tape (default=0)
NGOUT2 = 31 # unit for output gout tape (default=0)
IGN = 17 # neutron group structure option (corresponding to Vitamin J)
IGG = 0 # gamma group structure option
IWT = 11 # weight function option (corresponding to Vitamin E)
LORD = 0 # Legendre order
NTEMP = 1 # number of temperatures (default=1)
NSIGZ = 1 # number of sigma zeroes (default=1)
IPRINT = 1 # long print option (0/1=minimum/maximum) --(default=1)
ISMOOTH = 1 # switch on/off smoother operation (1/0, default=1=on)
TEMP = 293.16 # temperature in Kelvin
SIGZ = 0 # sigma zero values (including infinity)
MFD = 3 # file to be processed
MATD = 0 # next mat number to be processed
DIR = establish_directory()

# Dictionary of elements in the Periodic Table
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

def format_card(card_number, card_content, MTs):
    """
    Format individual "cards" for the NJOY input cards to run with GROUPR.
        Formatting and terminology based on the NJOY user manual:
        (https://github.com/njoy/NJOY2016-manual/raw/master/njoy16.pdf)

    Arguments:
        card_number (int): Individual identifier of "card" in input "deck".
        card_content (list): Values to be written on each individual "card".
        MTs (list of int): List of reaction types (MT's) present in the
            ENDF/PENDF files.
    
    Returns:
        card_str (str): Concatenated string of an individual "card's"
            contents. 
    """
    
    # Initialize string and concatenate contents of the card with it
    card_str = ''
    gen_str = ' ' + ' '.join(map(str, card_content))
    if card_number == 9:
        card_str = ' ' + '/\n '.join(card_content) + '/\n'
    elif card_number == 4:
        card_str += gen_str + '\n'
    else:
        card_str += gen_str + '/\n'
    return card_str

def groupr_input_file_format(matb, MTs, element, A, mt_dict):

    """"
    Format the input "deck" to run NJOY from individually formatted cards.
        Most parameters are predefined as constants, as necessary for a GROUPR
        run to write a group-wise file (GENDF) based on the ENDF and PENDF
        files with a Vitamin-J 175 group structure and a Vitamin-E weighting
        function. Other parameters are isotope specific, such as "matb", which
        corresponds to the NJOY naming convention for the material ID, which
        are separately extracted from the ENDF file to be referenced.

        Formatting and terminology based on the NJOY user manual:
        (https://github.com/njoy/NJOY2016-manual/raw/master/njoy16.pdf)

    Arguments:
        matb (int): Unique material ID extracted from the ENDF base file.
        MTs (list of int): List of reaction types (MT's) present in the
            ENDF/PENDF files.
        element (str): Chemical symbol for element of interest.
        A (str or int): Mass number for selected isotope.
            If the target is an isomer, "m" after the mass number,
            so A must be input as a string.
        mt_dict (dict): Reference dictionary containing reaction information
            for each MT number pre-defined in the ENDF manual.
            (https://www.oecd-nea.org/dbdata/data/manual-endf/endf102_MT.pdf)
    
    Returns:
        cards (dict): Dictionary containing each "card" identified by its card
            number.
    """

    cards = {}
    
    cards[1] = [NENDF, NPEND, NGOUT1, NGOUT2]
    # matb -- (already defined) -- material to be processed
    cards[2] = [matb, IGN, IGG, IWT, LORD, NTEMP, NSIGZ, IPRINT]
    Z = str(elements[element]).zfill(2)
    title = f'"{Z}-{element}-{A} for TENDL 2017"'
    cards[3] = [title]
    cards[4] = [TEMP]
    cards[5] = [SIGZ]
    mtd = MTs # sections to be processed
    cards[9] = []
    for MT in MTs:
        # Description of section to be processed
        mtname = mt_dict[str(MT)]['Reaction'] 
        card9_line = f'{MFD} {MT} "{mtname}"'
        cards[9].append(card9_line)
    cards[10] = [MATD]

    return cards

def groupr_input_file_writer(cards, MTs):
    """"
    Write out the NJOY GROUPR input card by formatting each card line by line.

    Arguments:
        cards (dict): Dictionary containing each "card" identified by its card
            number.
        MTs (list of int): List of reaction types (MT's) present in the
            ENDF/PENDF files.
    
    Returns:
        None
    """

    # Write the input deck to the groupr.inp file
    with open(f'{DIR}/groupr.inp', 'w') as f:
        f.write('groupr\n')
        for card_num, card in cards.items():
            f.write(format_card(card_num, card, MTs))
        f.write(' 0/\nstop')

def set_gendf_saving(save_directory, element, A):
    """
    Establish the save path for the GENDF file in the desired directory.

    Arguments:
        save_directory (str): Folder in which to save the GENDF file.
        element (str): Chemical symbol for element of interest.
        A (str or int): Mass number for selected isotope.
    
    Returns:
        gendf_path (str): File path to the newly created GENDF file.
    """

    # Create save_directory if it does not already exist
    save_directory = Path(save_directory)
    if not save_directory.exists():
        save_directory.mkdir(parents = True)

    gendf_path = f'{save_directory}/tendl_2017_{element}{A.zfill(3)}.gendf'
    return gendf_path

def text_insertion(string, identifier, new_text, file_lines):
    index = string.rfind(identifier)
    line_number = string[:index].count('\n')
    file_lines.insert(line_number + 1, new_text)
    return file_lines

def ensure_gendf_markers(gendf_path, matb):
    """
    Edit the GENDF files produced from an NJOY GROUPR run to include file and
    section records that are not automatically written out to the file by
    NJOY. Missing these records will not cause errors, but will raise
    messages when they are read by ENDFtk, which expects these markers, so
    this method ensures that they are present. Edits will only be made if
    the SEND record for MF1 or the FEND record for MF3 are not present.
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
    mf1_SEND_RECORD = f'{whitespace}{matb} 1  099999'
    mf3_FEND_RECORD = f'{whitespace}{matb} 0  0    0'

    with open(gendf_path, 'r') as gendf_file:
        file_str = gendf_file.read()
    file_lines = file_str.splitlines()

    updates = [
        (mf3_identifier, mf3_FEND_RECORD),
        (mf1_identifier, mf1_SEND_RECORD)
    ]

    for identifier, new_line in updates:
        last_line_index = file_str.rfind(identifier)
        line_number = file_str[:last_line_index].count('\n')
        file_lines.insert(line_number + 1, new_line)

    new_file_str = '\n'.join(file_lines) + '\n'

    with open(gendf_path, 'w') as gendf_file:
        gendf_file.write(new_file_str)

def run_njoy(cards, element, A, matb):
    """
    Use subprocess to run NJOY given a pre-written input card to convert a
        pair of ENDF and PENDF files to a GENDF file and save it locally.
    
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

    # Define the input files
    INPUT = f'{DIR}/groupr.inp'
    OUTPUT = f'{DIR}/groupr.out'

    # Run NJOY
    result = subprocess.run(['njoy'], input=open(INPUT).read(),
                            text=  True, capture_output=True)
    with open(OUTPUT, 'w') as output_file:
        output_file.write(result.stdout)

    # If the run is successful, log out the output
    # and make a copy of the file as a .GENDF file
    if result.stderr == '':
        output = subprocess.run(['cat', 'output'],
                                capture_output=True, text = True)
        title = cards[3][0][1:-1]
        title_index = output.stdout.find(title)
        logger.info(
            'Selected output of the latest NJOY run: \n'
            f'\n{output.stdout[:title_index + len(title)]}\n'
        )

        save_directory = f'{DIR}/gendf_files'
        gendf_path = set_gendf_saving(save_directory, element, A)
        subprocess.run(['cp', 'tape31', gendf_path])
        ensure_gendf_markers(gendf_path, matb)

        return gendf_path, save_directory
    else:
        logger.error(result.stderr)

def njoy_file_cleanup(output_path = f'{DIR}/njoy_ouput'):
    """
    Clean up repository from unnecessary intermediate files from NJOY run.

    Arguments:
        output_path (str, optional): The save path for the NJOY output.
            Defaults to f'{DIR}/njoy_output', which will save the file in the
            same directory as all other saved files from the script run.
    
    Returns:
        None
    """

    njoy_files = [f'{DIR}/groupr.inp', f'{DIR}/groupr.out',
                  'tape20', 'tape21', 'tape31']
    for file in njoy_files:
        if file == f'{DIR}/groupr.out':
            with open(file, 'r') as f:
                groupr_out = f.read()
            logger.info(
                f'GROUPR run-specific analytics: \n {groupr_out}'
            )
            
        subprocess.run(['rm', file])
    subprocess.run(['mv', 'output', output_path])
    logger.info(f'Full NJOY output can be found at {output_path}')