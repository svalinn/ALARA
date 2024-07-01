# Import packages
import gendf_tools as GENDFtk
import pandas as pd
from logging_config import logger
import groupr_tools as GRPRtk
import sys
import argparse

# Define an argument parser
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

def main():
    """
    Main method when run as a command line script.
    """
    # Load MT table
    # Data for MT table collected from 
    # https://www.oecd-nea.org/dbdata/data/manual-endf/endf102_MT.pdf
    mt_dict = GENDFtk.read_csv('mt_table.csv')
    
    # Parse command line arguments
    args = fendl_args()

    # Set conditionals for local file input 
    if args.method == 'I':
        gendf_path = args.local_path
        M = args.isomer
        pKZA = GENDFtk.gendf_pkza_extract(gendf_path, M = M.upper())
        matb, MTs, file_obj = GENDFtk.endf_specs(gendf_path, 'gendf')
    
    # Set conditionals for file download
    elif args.method == 'D':
        element = args.element
        A = args.A
        # Use NJOY GROUPR to convert the isomer's TENDL 2017 data to a GENDF file

        # Download ENDF and PENDF files for the isomer
        endf_path = GRPRtk.tendl_download(element, A, 'endf')
        pendf_path = GRPRtk.tendl_download(element, A, 'pendf')

        # Extract necessary MT and MAT data from the ENDF file
        matb, MTs = GENDFtk.endf_specs(endf_path, 'endf')
        
        # Write out the GROUPR input file
        card_deck = GRPRtk.groupr_input_file_format(matb, MTs, element, A, mt_dict)
        GRPRtk.groupr_input_file_writer(card_deck, MTs)

        # Run NJOY with GROUPR to create a GENDF file for the isomer
        gendf_path = GRPRtk.run_njoy(card_deck, element, A)

        # Save pKZA value
        M = 'M' if 'm' in A else None
        pKZA = GENDFtk.gendf_pkza_extract(gendf_path, M = M)

        # Recalibrate MT list after GENDF conversion
        matb, MTs, file_obj = GENDFtk.endf_specs(gendf_path, 'gendf')

        # Clean up repository from unnecessary intermediate files from GROUPR run
        GRPRtk.njoy_file_cleanup()

    logger.info(f"GENDF file path: {gendf_path}")
    logger.info(f"Parent KZA (pKZA): {pKZA}")
    logger.info(f'MTs: {MTs}')

    # Extract and save data for each MT
    gendf_data = GENDFtk.iterate_MTs(MTs, file_obj, mt_dict, pKZA)

    # Save to CSV
    gendf_data.to_csv('gendf_data.csv', index=False)
    logger.info("Saved gendf_data.csv")

# Execute main() function based on arguments
if __name__ == '__main__':
    main()