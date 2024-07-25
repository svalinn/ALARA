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

def groupr_static_template():

    """
    Formulate a static string that serves as a template for an NJOY input file
        used to run the GROUPR module. The filled-in parameters are predefined
        constants, as are necessary for a GROUPR run to write a group-wise 
        nuclear data file (GENDF) based on the ENDF and PENDF files with a 
        Vitamin-J 175 groupr structure and a Vitamin-E weighting function.
        There are certain components that need to be added separately to the
        formattted string, including the material ID, as the 0th element of
        Card 2, the title for Card 3, and the reaction data for Card 9. These
        values are isotope and file specific, so are not included in this
        general template.

    Arguments:
        None
    Returns:
        cards (dict): Dictionary containing each "card" identified by its card
            number, containing the static input data.
    """

    cards = {}
    
    cards[1] = [NENDF, NPEND, NGOUT1, NGOUT2]
    # Need to add mat_id as 0th element
    cards[2] = [IGN, IGG, IWT, LORD, NTEMP, NSIGZ, IPRINT]
    # Need to add title
    cards[3] = []
    cards[4] = [TEMP]
    cards[5] = [SIGZ]
    # Need to add specific reactions
    cards[9] = []
    cards[10] = [MATD]

    return cards