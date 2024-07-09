# Import packages
import subprocess
from logging_config import logger, LoggerWriter

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
        MTs (list): List of reaction types (MT's) present in the ENDF/PENDF files.
    
    Returns:
        card_str (str): Concatenated string of an individual "card's" contents. 
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
        Most parameters are predefined as constants, as necessary for a GROUPR run
        to write a group-wise file (GENDF) based on the ENDF and PENDF files
        with a Vitamin-J 175 group structure and a Vitamin-E weighting function.
        Other parameters are isotope specific, such as "matb", which corresponds
        to the NJOY naming convention for the material ID, which are separately
        extracted from the ENDF file to be referenced.

        Formatting and terminology based on the NJOY user manual:
        (https://github.com/njoy/NJOY2016-manual/raw/master/njoy16.pdf)

    Arguments:
        matb (int): Unique material ID extracted from the ENDF base file.
        MTs (list): List of reaction types (MT's) present in the ENDF/PENDF files.
        element (str): Chemical symbol for element of interest.
        A (str or int): Mass number for selected isotope.
            If the target is an isomer, "m" after the mass number,
            so A must be input as a string.
        mt_dict (dict): Reference dictionary containing reaction information
            for each MT number pre-defined in the ENDF manual.
            (https://www.oecd-nea.org/dbdata/data/manual-endf/endf102_MT.pdf)
    
    Returns:
        cards (dict): Dictionary containing each "card" identified by its card number.
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
        mtname = mt_dict[str(MT)] # description of section to be processed
        card9_line = f'{MFD} {MT} "{mtname}"'
        cards[9].append(card9_line)
    cards[10] = [MATD]

    return cards

def groupr_input_file_writer(cards, MTs):
    """"
    Write out the NJOY GROUPR input card by formatting each card line by line.

    Arguments:
        cards (dict): Dictionary containing each "card" identified by its card number.
        MTs (list): List of reaction types (MT's) present in the ENDF/PENDF files.
    
    Returns:
        None
    """

    # Write the input deck to the groupr.inp file
    with open('groupr.inp', 'w') as f:
        f.write('groupr\n')
        for card_num, card in cards.items():
            f.write(format_card(card_num, card, MTs))
        f.write(' 0/\nstop')

def run_njoy(cards, element, A):
    """
    Use subprocess to run NJOY given a pre-written input card to convert a pair
        of ENDF and PENDF files to a GENDF file and save it locally.
    
    Arguments:
        cards (dict): Dictionary containing each "card" identified by its card number.
        element (str): Chemical symbol for element of interest.
        A (str or int): Mass number for selected isotope.
            If the target is an isomer, "m" after the mass number,
            so A must be input as a string.
    
    Returns:
        gendf_path (str): File path to the newly created GENDF file. 
    """

    # Define the input files
    INPUT = 'groupr.inp'
    OUTPUT = 'groupr.out'

    # Run NJOY
    result = subprocess.run(['njoy'], input=open(INPUT).read(), text=  True, capture_output=True)
    with open(OUTPUT, 'w') as output_file:
        output_file.write(result.stdout)

    # If the run is successful, log out the output and make a copy of the file as a .GENDF file
    if result.stderr == '':
        output = subprocess.run(['cat', 'output'], capture_output=True, text = True)
        title = cards[3][0][1:-1]
        title_index = output.stdout.find(title)
        logger.info(f'\n{output.stdout[:title_index + len(title)]}\n')

        gendf_path = f'./gendf_files/tendl_2017_{element}{A}.gendf'
        subprocess.run(['cp', 'tape31', gendf_path])
        return gendf_path
    else:
        logger.error(result.stderr)

def njoy_file_cleanup(output_path = 'njoy_ouput'):
    """
    Clean up repository from unnecessary intermediate files from NJOY run.

    Arguments:
        output_path (str): The path where the automated NJOY output will be saved.
            Defaults to 'njoy_output'.
    
    Returns:
        None
    """

    njoy_files = ['groupr.inp', 'groupr.out', 'tape20', 'tape21', 'tape31']
    for file in njoy_files:
        if file == 'groupr.out':
            with open(file, 'r') as f:
                groupr_out = f.read()
            logger.info(groupr_out)
            
        subprocess.run(['rm', file])
    subprocess.run(['mv', 'output', output_path])
    logger.info(f'Full NJOY output file readout can be found at {output_path}')