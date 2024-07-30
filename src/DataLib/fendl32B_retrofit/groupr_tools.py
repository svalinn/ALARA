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

# Create input file general template
njoy_input = Template(Template(
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
).safe_substitute(
    NENDF = NENDF, NPEND = NPEND, NGOUT1 = NGOUT1, NGOUT2 = NGOUT2, # Card 1
    IGN = IGN, IGG = IGG, IWT = IWT, LORD = LORD,                   # Card 2
    NTEMP = NTEMP, NSIGZ = NSIGZ, IPRINT = IPRINT,                  # Card 2
    TEMP = TEMP,                                                    # Card 4
    SIGZ = SIGZ,                                                    # Card 5
    MATD = MATD                                                     # Card 10
))