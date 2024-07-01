# Import packages
import tendl_preprocessing as tpp
import activation_analysis
from logging_config import logger
import groupr_tools
import sys
import argparse

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
        None
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
        parser_I = subparsers.add_parser('I', help='Local file input.')
        parser_I.add_argument('--local-path',
                            required=True,
                            help='Path to the local GENDF file.')
        parser_I.add_argument('--isomer', '-m',
                              required=False,
                              default=None,
                              help = 'Isomeric state of the element.')
        parser_D = subparsers.add_parser('D', help='Download TENDL/PENDF files.')
        parser_D.add_argument('--element', '-e',
                            required=True,
                            help= 'Chemical symbol for selected element (i.e. Ti).')
        parser_D.add_argument('--A', '-a',
                            required=True,
                            help='Mass number for selected isotope (i.e. 48). If the target is an isomer, "m" after the mass number (i.e. 48m).')

        args = parser.parse_args()
        return args
    
    finally:
        # Restore stdout and stderr to the logger
        sys.stdout = original_stdout
        sys.stderr = original_stderr

def fendl3_2b_retrofit():
    """
    Main method when run as a command line script.
    """
    
    args = fendl_args()

    # Load MT table
    mt_dict = tpp.read_csv('mt_table.csv')

    # Set conditionals for local file input 
    if args.method == 'I':
        gendf_path = args.local_path
        M = args.isomer
        pKZA = tpp.extract_gendf_pkza(gendf_path, M = M.upper())
        matb, MTs, file_obj = tpp.extract_endf_specs(gendf_path, 'gendf')
    
    # Set conditionals for file download
    elif args.method == 'D':
        element = args.element
        A = args.A

        endf_path = tpp.download_tendl(element, A, 'endf')
        pendf_path = tpp.download_tendl(element, A, 'pendf')

        material_id, MTs = tpp.extract_endf_specs(endf_path, 'endf')
        
        card_deck = groupr_tools.groupr_input_file_format(material_id, MTs, element, A, mt_dict)
        groupr_tools.groupr_input_file_writer(card_deck, MTs)

        gendf_path = groupr_tools.run_njoy(card_deck, element, A)

        M = 'M' if 'm' in A else None
        pKZA = tpp.extract_gendf_pkza(gendf_path, M = M)

        # Recalibrate MT list after GENDF conversion
        matb, MTs, file_obj = tpp.extract_endf_specs(gendf_path, 'gendf')

        groupr_tools.njoy_file_cleanup()

    logger.info(f"GENDF file path: {gendf_path}")
    logger.info(f"Parent KZA (pKZA): {pKZA}")
    logger.info(f'MTs: {MTs}')

    # Extract and save data for each MT
    gendf_data = activation_analysis.iterate_MTs(MTs, file_obj, mt_dict, pKZA)

    # Save to CSV
    gendf_data.to_csv('gendf_data.csv', index=False)
    logger.info("Saved gendf_data.csv")

# Execute main() function based on arguments
if __name__ == '__main__':
    fendl3_2b_retrofit()