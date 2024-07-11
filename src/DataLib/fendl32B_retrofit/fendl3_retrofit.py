# Import packages
import tendl_preprocessing as tpp
import activation_analysis
from logging_config import logger
import groupr_tools
import sys
import argparse
import asyncio
import pandas as pd
import reaction_data as rxd

def fendl_args():
    """
    Configure Argument Parser for the FENDL 3.2-b/TENDL 2017 retroffiting,
        including options for user input of GENDF files that have already
        been processed using the NJOY GROUPR module or downloading and
        processing TENDL/PENDF files from the TENDL 2017 database.

        Note: If the user opts to select the "Loclal file input" option,
        they must ensure that the GENDF file is properly formatted and
        contains activation, and not transport data, themselves.
    
    Arguments:
        None
    
    Returns:
        args (argparse.Namespace): Argparse object that contains the user
            specified arguments for executing the program.
    """

    # Temporarily reset stdout and stderr to the defaults
    # so that arg messages (i.e. --help) print out to terminal
    original_stdout = sys.stdout
    original_stderr = sys.stderr

    try:
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__

        parser = argparse.ArgumentParser()

        # Subparsers for 'I' and 'D'
        subparsers = parser.add_subparsers(dest='method', required=True)
        parser_I = subparsers.add_parser('I', help='''Local file input.
                                         Note: This option should only be 
                                         selected if the user already has
                                         properly formatted GENDF activation 
                                         files that have been processed using 
                                         the NJOY GROUPR module for a
                                        Vitmain-J group structure with a
                                         Vitamin-E weight function.''')
        parser_I.add_argument('--paths', '-p',
                            required=True,
                            nargs= '+',
                            help='Path to the local GENDF file.')
        parser_D = subparsers.add_parser('D', help='''Download TENDL/PENDF files
                                         from the TENDL 2017 neutron activation
                                         database.''')
        parser_D.add_argument('--element', '-e',
                            required=True,
                            help= 'Chemical symbol for selected element (i.e. Ti).')
        parser_D.add_argument('--A', '-a',
                            required=True,
                            help= '''Mass number for selected isotope (i.e. 48).
                                        If the target is an isomer, type
                                        "m" after the mass number (i.e. 48m).
                                        To automatically iterate over all of
                                        the isotopes for the target element,
                                        select "all" as the option for --A.
                                        ''')

        args = parser.parse_args()
        return args
    
    finally:
        # Restore stdout and stderr to the logger
        sys.stdout = original_stdout
        sys.stderr = original_stderr


def handle_local_file_input(mt_dict, cumulative_data, gendf_paths = None, args = None):
    """
    Method for extracting and analyzing data from preprocessed GENDF files
        that the user already has saved locally. Called when the argument
        for the main function -- fendl3_2b_retrofit() -- is specified as 'I'.
    
    Arguments:
        args (argparse.Namespace): Argparse object that contains the user
            specified arguments for executing the program.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv 
    """

    # Format gendf_paths parameter
    if gendf_paths is None:
        gendf_paths = args.paths
    elif type(gendf_paths) is not list:
        gendf_paths = [gendf_paths]

    for gendf_path in gendf_paths:
        # Extract fundamental data from the GENDF file
        pKZA = tpp.extract_gendf_pkza(gendf_path)
        matb, MTs, file_obj = tpp.extract_endf_specs(gendf_path, 'gendf')

        logger.info(f"GENDF file path: {gendf_path}")
        logger.info(f"Parent KZA (pKZA): {pKZA}")
        logger.info(f'MTs: {MTs}')

        # Extract and save specific data for each MT
        gendf_data = activation_analysis.iterate_MTs(MTs, file_obj, mt_dict, pKZA)
        cumulative_data = pd.concat([cumulative_data, gendf_data], ignore_index= True)
    return cumulative_data

def handle_download_file(args, mt_dict, cumulative_df):
    """
    Method for downloading ENDF/PENDF files from the TENDL 2017 database,
        using the NJOY GROUPR module to convert these to a group-wise file,
        and then extracting and analyzing the resultant data. Called when
        the argument for the main function -- fendl3_2b_retrofit() --
        is specified as 'D'.
    
    Arguments:
        args (argparse.Namespace): Argparse object that contains the user
            specified arguments for executing the program.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv 
    """

    # Establish parameters from the user arguments
    element = args.element
    A = args.A
    if A == 'all':
        A_vals = asyncio.run(tpp.identify_tendl_isotopes(element))
        logger.info(f'All isotopes of {element} (by mass number): {A_vals}')
    else:
        A_vals = [A]

    # Iterate over all isotopes/isomers, as specified by arguments
    gendf_paths = []
    for A in A_vals:
        endf_path = tpp.download_tendl(element, A, 'endf')
        pendf_path = tpp.download_tendl(element, A, 'pendf')

        material_id, MTs = tpp.extract_endf_specs(endf_path, 'endf')
        
        card_deck = groupr_tools.groupr_input_file_format(material_id, MTs,
                                                        element, A, mt_dict)
        groupr_tools.groupr_input_file_writer(card_deck, MTs)

        gendf_path = groupr_tools.run_njoy(card_deck, element, A, material_id)
        gendf_paths.append(gendf_path)
        
        groupr_tools.njoy_file_cleanup()
        logger.info(f'Finished iterating for {element}-{A} \n \n')
    
    return gendf_paths

def fendl3_2b_retrofit():
    """
    Main method when run as a command line script.
    """

    # Initialize arguments, DataFrame to store data, and load in MT reference
    args = fendl_args()
    gendf_paths = []
    cumulative_data = rxd.initialize_dataframe()
    mt_dict = rxd.process_mt_table('mt_table.csv')

    # Execute file handling
    if args.method == 'D':
        gendf_paths = handle_download_file(args, mt_dict, cumulative_data)
        print(gendf_paths)
        args = None
    cumulative_data = handle_local_file_input(mt_dict,
                                              cumulative_data,
                                              gendf_paths = gendf_paths,
                                              args = args)
        
    # Save to CSV
    cumulative_data.to_csv('gendf_data.csv', index=False)
    logger.info("Saved gendf_data.csv")

# Execute main() function based on arguments
if __name__ == '__main__':
    fendl3_2b_retrofit()