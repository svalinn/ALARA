import subprocess
from string import Template
from pathlib import Path

#--------------- Running Single Parent Element Simulation(s) -----------------

INPUT = 'alara.inp'

# Adapted from ALARA/examples/singleElement.ala
alara_input = Template(
'''
geometry rectangular
dimension	x
		0.0
        1       5.0
end
mat_loading
        inner_zone1  mix1
end
material_lib ../../data/matlib.sample
element_lib ../../data/nuclib.std
data_library alaralib $datalib
mixture mix1
        element $element              1.0     1.00
end
flux flux_1 ../../examples/ref_flux_files/fluxfnsfIBfw_518MW.txt  1.0   0   default
schedule 2_year
	2 y  flux_1  steady_state  0 s
end
pulsehistory steady_state
	1	0 s
end
dump_file dump_singleElement
cooling
	1e-5 y
	1e-2 y
	1 y
	100 y
	10000 y
end
output interval
        units Bq kg
        number_density
        specific_activity
	total_heat
	dose contact $datalib ../../data/ANS6_4_3
end
## 
truncation  1e-7
'''
)

def fill_alara_template(element, datalib):
    '''
    Substitute in the specific single parent element and path to a
        pre-converted ALARA binary library, such as that for either FENDL2 or
        ALARAJOY-processed FENDL3, to a template containing a generalized
        ALARA input file text for a simple single parent element simulation.
    Arguments:
        element (str): Single parent element to be irradiated.
        datalib (str): Path to the binary library.
    
    Returns:
        alara_input (str): String template with appropriate variables
            substituted in for Template identifiers.
    '''

    return alara_input.substitute(element=element, datalib=datalib)

def write_alara_input_file(template):
    '''
    Write out the ALARA input card from the prefilled template.
    Arguments:
        template (str): String template with appropriate variables substituted
            in for Template identifiers.
    Returns:
        None
    '''

    with open(INPUT, 'w') as f:
        f.write(template)

def run_alara(element, libname):
    '''
    Invoke subprocess.run() to run ALARA for the single parent element
        irradiation simulation. Specify destination for ALARA tree file and
        capture stdout to an output file to be read by
        alara_pandas_parser.parse_tables().
    Arguments:
        element (str): Single parent element to be irradiated.
        libname (str): Name of the source data library (i.e. fendl2, fendl3,
            etc.)
    Returns:
        output (str): Path to the ALARA redirected ALARA stdout formatted as a
            text file.
    '''

    filename_base = f'{element}_{libname}'
    output = f'{filename_base}.out'
    Path(output).unlink(missing_ok=True)
    with open(output, 'w') as outfile:
        subprocess.run(
            ['alara', '-t', f'{filename_base}.tree', '-v', '3', INPUT],
            stdout=outfile,
            stderr=subprocess.STDOUT,
            check=True
        )

    return output

