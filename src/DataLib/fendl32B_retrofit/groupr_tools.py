# Import packages
import urllib
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

# Define a function to download the .tendl file given specific user inputs to for element and mass number
def tendl_download(element, A, filetype, save_path = None):
    # Ensure that A is properly formatted
    A = str(A).zfill(3 + ('m' in A))

    # Define general URL format for files in the TENDL database
    tendl_gen_url = 'https://tendl.web.psi.ch/tendl_2017/neutron_file/'

    # Create a dictionary to generalize formatting for both ENDF and PENDF files
    file_handling = {'endf' : {'ext': 'tendl', 'tape_num': 20},
                     'pendf' : {'ext': 'pendf', 'tape_num': 21}}
    
    # Construct the filetype and isotope specific URL
    isotope_component = f'{element}/{element}{A}/lib/endf/n-{element}{A}.'
    ext = file_handling[filetype.lower()]['ext']
    download_url = tendl_gen_url + isotope_component + ext

    # Define a save path for the file if there is not one already specified
    if save_path is None:
        save_path = f'tape{file_handling[filetype.lower()]["tape_num"]}'

    # Check if the file exists
    try:
        urllib.request.urlopen(download_url)
    except urllib.error.URLError as e:
        file_not_found_code = 404
        if str(file_not_found_code) in str(e):
            raise FileNotFoundError()

    # Download the file using urllib
    with urllib.request.urlopen(download_url) as f:
        temp_file = f.read().decode('utf-8')
    
    # Write out the file to the save_path
    with open(save_path, 'w') as f:
        f.write(temp_file)

    return save_path

# Define a function to format GROUPR input cards
def format_card(card_number, card_content, MTs):
    card_str = ''
    gen_str = ' ' + ' '.join(map(str, card_content))
    if card_number == 9:
        card_str = ' ' + '/\n '.join(card_content) + '/\n'
    elif card_number == 4:
        card_str += gen_str + '\n'
    else:
        card_str += gen_str + '/\n'
    return card_str

# Define a function to create the GROUPR input file
def groupr_input_file_format(matb, MTs, element, A, mt_table):

    cards = {}

    # Set Card 1
    cards[1] = [NENDF, NPEND, NGOUT1, NGOUT2]

    # Set Card 2
    # matb -- (already defined) -- material to be processed
    cards[2] = [matb, IGN, IGG, IWT, LORD, NTEMP, NSIGZ, IPRINT]

    # Set Card 3
    Z = str(elements[element]).zfill(2)
    title = f'"{Z}-{element}-{A} for TENDL 2017"'
    cards[3] = [title]

    # Set Card 4
    cards[4] = [TEMP]

    # Set Card 5
    cards[5] = [SIGZ]

    # Set Card 9
    mtd = MTs # sections to be processed
    cards[9] = []
    for mt in MTs:
        index = mt_table['MT'].index(str(mt))
        mtname = mt_table['Reaction'][index] # description of section to be processed
        card9_line = f'{MFD} {mt} "{mtname}"'
        cards[9].append(card9_line)

    # Set Card 10
    cards[10] = [MATD]

    return cards

# Define a function to write out the GROUPR input file
def groupr_input_file_writer(cards, MTs):
    # Write the input deck to the groupr.inp file
    with open('groupr.inp', 'w') as f:
        f.write('groupr\n')
        for card_num, card in cards.items():
            f.write(format_card(card_num, card, MTs))
        f.write(' 0/\nstop')

# Define a function to execute NJOY bash script
def run_njoy(cards, element, A):
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

        gendf_path = f'tendl_2017_{element}{A}.gendf'
        subprocess.run(['cp', 'tape31', gendf_path])
        return gendf_path
    else:
        logger.error(result.stderr)