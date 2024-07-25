# Import packages
from string import Template

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

# Define a dictionary template for GROUPR
cards = {
    1  :                         [NENDF, NPEND, NGOUT1, NGOUT2],
    2  : ['$mat_id', IGN, IGG, IWT, LORD, NTEMP, NSIGZ, IPRINT],
    3  :                                             ['$title'],
    4  :                                                 [TEMP],
    5  :                                                 [SIGZ], 
    9  :                                         ['$reactions'],
    10 :                                                 [MATD]
}

def format_card(card_number, card_content):
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

def establish_static_template():

    """
    Formulate a static string that serves as a template for an NJOY input file
        used to run the GROUPR module. The filled-in parameters are predefined
        constants, as are necessary for a GROUPR run to write a group-wise 
        nuclear data file (GENDF) based on the ENDF and PENDF files with a 
        Vitamin-J 175 groupr structure and a Vitamin-E weighting function.
        There are certain variable components whose values can be substituted
        in for the current fillers using the string.substitute() method.

    Arguments:
        None
    Returns:
        input_template (string.Template): A static string template containing
            the general structure of the NJOY input file to run GROUPR,
            $identifiers for substitutable items
            (https://docs.python.org/3.4/library/string.html#template-strings)
        """

    input_string = 'groupr\n'
    for card_num, card in cards.items():
        input_string += format_card(card_num, card)
    input_string += ' 0/\nstop'
    input_template = Template(input_string)

    return input_template