import subprocess
import pandas as pd
from string import Template
from pathlib import Path
import sys
sys.path.append('..')
from alara_output_parser import parse_tables

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

#---------- Loading Results of Simulation(s) into a Data Structure -----------

def make_entry(datalib, variable, unit, data):
    '''
    Construct a dictionary entry for a single Pandas DataFrame and its
        associated metadata for updating into a larger data dictionary.
    Arguments:
        datalib (str): Data source for the DataFrame (i.e. fendl2,
            ALARAJOY-fendl3, etc.).
        variable (str): Dependent variable evaluated in DataFrame (i.e. Number
            Density, Specific Activity, etc.).
        unit (str): Associated units for the above variable (i.e. atoms/kg, 
            Bq/kg, etc.).
        data (pandas.core.frame.DataFrame): Pandas DataFrame described by
            the above metadata.
    Returns:
        entry (dict): Single data entry for dictionary containing potentially
            multiple dataframes and associated metadata.
    '''

    return {
        f'{datalib} {variable}': {
            'Data Source': datalib,
            'Variable': variable,
            'Unit': unit,
            'Data': data
        }
    }

def process_data(
        data_source,
        inp_datalib=None,
        inp_variable=None,
        inp_unit=None
        ):
    '''
    Flexibly create a dictionary of subdictionaries containing Pandas
        DataFrames with associated metadata containing ALARA output data for
        different variables. Allows for processing of either existing
        DataFrames, with the requirement that the user provide the relevant
        data source, evaulated variable, and its relevant units to be packaged
        into a data dictionary. Alternatively, this function can directly
        read in an ALARA output file and parse all tables and their metadata
        internally.
    Arguments:
        data_source (dict or pandas.core.frame.DataFrame): ALARA output data.
            If parsing directly from ALARA output files, data_source must be
            formatted as a dictionary of the form:
            data_source = {
                Data Library 1 : path/to/output/file/for/data/library1,
                Data Library 2 : path/to/output/file/for/data/library2
            }
            If processing a preexisting DataFrame, then data_source is just
            the DataFrame itself.
        inp_datalib (str or None, optional): Data source of the selected
            DataFrame. Required if processing a preexisting DataFrame;
            irrelevant if parsing directly from ALARA output files.
            (Defaults to None)
        inp_variable (str or None, optional): Evaluated variable for the
            selected DataFrame. Required if processing a preexisting
            DataFrame; irrelevant if parsing directly from ALARA output files.
            (Defaults to None)
        inp_unit (str or None, optional): Appropriate unit for the evaluated
            variable for the selected DataFrame. Required if processing a
            preexisting DataFrame; irrelevant if parsing directly from ALARA
            output files.
            (Defaults to None)
    Returns:
        dfs (list of dicts): List of dictionaries containing ALARA output
            DataFrames and their metadata, of the form:
            df_dict = {
                'Data Source' : (Either 'fendl2' or 'fendl3'),
                'Variable'    : (Any ALARA output variable, dependent on ALARA
                                 run parameters),
                'Unit'        : (Respective unit matching the above variable),
                'Data'        : (DataFrame containing ALARA output data for
                                 the given data source, variable, and unit)
            }
    '''

    dfs = {}

    if type(data_source) == pd.core.frame.DataFrame:
        dfs.update(
            make_entry(inp_datalib, inp_variable, inp_unit, data_source)
        )
        return dfs

    for datalib, output_path in data_source.items():
        output_tables = parse_tables(output_path)
        for key, data in output_tables.items():
            variable_w_unit, _ = key.split(' - ')
            variable, unit = variable_w_unit.split(' [')
            dfs.update(make_entry(datalib, variable, unit.strip(']'), data))

    return dfs