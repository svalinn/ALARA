# Import packages
from string import Template

# Define constants
NENDF = 20                                               # unit for endf tape
NPEND = 21                                              # unit for pendf tape
NGOUT1 = 0                             # unit for input gout tape (default=0)
NGOUT2 = 31                           # unit for output gout tape (default=0)
IGN = 17        # neutron group structure option (corresponding to Vitamin J)
IGG = 0                                        # gamma group structure option
IWT = 11                # weight function option (corresponding to Vitamin E)
LORD = 0                                                     # Legendre order
NTEMP = 1                                # number of temperatures (default=1)
NSIGZ = 1                                # number of sigma zeroes (default=1)
IPRINT = 1            # long print option (0/1=minimum/maximum) --(default=1)
ISMOOTH = 1            # switch on/off smoother operation (1/0, default=1=on)
TEMP = 293.16                                         # temperature in Kelvin
SIGZ = 0                             # sigma zero values (including infinity)
MFD = 3                                                # file to be processed
MATD = 0                                    # next mat number to be processed

# Create input file general template
njoy_template = Template(
    """
    groupr/
     $NENDF $NPEND $NGOUT1 $NGOUT2/
     $mat_id $IGN $IGG $IWT $LORD $NTEMP $NSIGZ $IPRINT/
     $title/
     $TEMP
     $SIGZ/
     $reactions
     $MATD
     0/
    stop
    """
)


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
        template (string.Template): Modified template with the material-
            specific information substituted in for the $identifiers.
    """

    Z = str(elements[element]).zfill(2)
    title = f'"{Z}-{element}-{A} for TENDL 2017"'

    card9_lines = []
    for i, MT in enumerate(MTs):
        mtname = mt_dict[str(MT)]['Reaction']
        card9_lines.append(f'{MFD} {MT} "{mtname}" /') 
    card9 = '\n '.join(card9_lines)
    
    return njoy_template.substitute(
        NENDF=NENDF, NPEND=NPEND, NGOUT1=NGOUT1, NGOUT2=NGOUT2, # Card 1
        mat_id=material_id, IGN=IGN, IGG=IGG, IWT=IWT,          # Card 2
        LORD=LORD, NTEMP=NTEMP, NSIGZ=NSIGZ, IPRINT=IPRINT,     # Card 2 cont.
        title=title,                                            # Card 3
        TEMP=TEMP,                                              # Card 4
        SIGZ=SIGZ,                                              # Card 5
        reactions=card9,                                        # Card 9
        MATD=MATD                                               # Card 10
    )

def write_njoy_input_file(template):
    """
    Write out the NJOY GROUPR input card from the prefilled template.

    Arguments:
        template (string.Template): A filled template containing all of the
            parameters to be written out to the NJOY input card.
    Returns:
        None
    """

    with open('groupr.inp', 'w') as f:
        f.write(template)