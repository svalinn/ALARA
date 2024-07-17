# Import packages
import tendl_preprocessing as tpp
from logging_config import logger
import groupr_tools
import argparse
import asyncio
import pandas as pd
import reaction_data as rxd
import subprocess
from pathlib import Path

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

    parser = argparse.ArgumentParser()

    parser.add_argument(
        '--paths', '-p', required = False, nargs='*',
        help= '''
            Path(s) to the local GENDF file(s) or the repository in which
            they are located.
                
            If left empty, then corresponding ENDF and
            PENDF files will be downloaded and processed into properly
            formatted GENDF files.
            ''')
    parser.add_argument(
        '--element', '-e', required=False, nargs='*',
        help= '''
            Chemical symbol(s) for selected element(s) (i.e. Ti).

            To iterate over all elements, either type "all" or leave empty.
            ''')
    parser.add_argument(
        '--mass_number', '-a', required= False, nargs = '*',
        help= '''Mass number for selected isotope (i.e. 48). If the target is
                 an isomer, type "m" after the mass number (i.e. 48m).
                 
                 To automatically iterate over all of the isotopes for the
                 target element, type "all" or leave empty.
              ''')

    args = parser.parse_args()
    return args

def prepare_ls_commands(args, gendf_dir, element):
    """
    Create a list of commands to be run with a subprocess.run() executable to
        be used to search for GENDF files in a given directory for a
        particular element. This can either be targetted for individual files
        specified by the mass number for an isotope, whose data is stored in
        a GENDF file, or all files for the element in the directory.

    Arguments:
        args (argparse.Namespace): Argparse object that contains user-declared
            parameters, from which the only one accessed is mass_number, to
            determine whether to prepare a search command for a pre-defined
            list of files or to search for any and all files for the element.
        gendf_dir (str): Path for the directory containing the relevant GENDF
            file(s).
        element (str): Chemical name of the target element.
    
    Returns:
        commands (list of str): List of all "ls" search commands to be run
            with the subprocess module, corresponding to the desired files to
            be processed.
    """

    commands = []
    A_vals = args.mass_number

    if not A_vals or A_vals == ['all']:
        ls_command = (f'ls {Path(gendf_dir) / f"*{element}*"}', None)
        commands.append(ls_command)
    else:
        for A in A_vals:
            A = A.zfill(3)
            ls_command = (f'ls {Path(gendf_dir) / f"*{element}{A}*"}', A)
            commands.append(ls_command)

    return commands

def run_ls_command(command):
    """
    Use subprocess.run() to execute an "ls" command to search for a particular
        GENDF file or multiple GENDF files. This function streamlines usage of
        subprocess by making it only necessary to input the searching command,
        without having to specify the other run() parameters.
    
    Arguments:
        command (str): Shell command to be used by subprocess.run() to search
            for a GENDF file or GENDF files using "ls".
        
    Returns:
        ls_output (list of str): List of the relative file paths to all of the
            GENDF files found from the "ls" command.
    """

    ls_output = subprocess.run(
        command,                    # Execute the ocmmand using subprocess.run
        shell=True,                 # Run the command in the shell    
        check=True,                 # Raise exception if returns non-zero code
        stdout=subprocess.PIPE,     # Capture the standard output
        stderr=subprocess.PIPE,     # Capture the standard error
        universal_newlines=True     # Return the output as a string, not bytes
        ).stdout.strip().split("\n")   

    return ls_output

def warn_missing_file(error, element, A):
    """
    Identify and log a warning if the file for a partiular isotope is not
        found when the run_ls_command() function is executed. Rather than
        raise an error to the terminal, it will just be logged as a warning
        so that if the user is searching for multiple elements and a suitable
        isotope exists for one but not the other, it will not cause the
        execution to fail, but rather just to make note of the case and warn
        the user to double check whether or not that file was supposed to
        exist.
    
    Arguments:
        error (subprocess.CalledProcessError): A subprocess error object that
            signifies that the execution of the command was unsuccessful and
            returns a non-zero exit status.
        element (str): Chemical name of the target element.
        A (str): Mass number of the target isotope, including the letter "m"
            at the end if the target is a nuclear isomer.
    
    Returns:
        None
    """

    if error.returncode == 2: # Error code for CalledProcessError

        logger.warning(error.stderr.strip())
        specific_message = f'{element}{A or ""}'

        warning_message = (
            f'If there is a GENDF file for {element}'
            f'{f" with mass number {A}" if A is not None else ""},'
            ' then make sure it is properly named and '
            f'formatted with the string "{specific_message}" '
            'included in the file\'s name.'
        )
        logger.warning(warning_message)
    else:
        raise

def search_files_for_element(args, gendf_dir, element):
    """
    Generate and iterate through the GENDF file search commands produced from
        prepare_ls_commands() to either store the file paths in a cumulative
        list or to log missing files and tally the number of warnings from a
        given run.

    Arguments:
        args (argparse.Namespace): Argparse object that contains user-declared
            parameters, which is read in the prepare_ls_commands() function.
        gendf_dir (str): Path for the directory containing the relevant GENDF
            file(s).
        element (str): Chemical name of the target element.

    Returns:
        element_gendf_paths (list of str): List containing all of the relative
            file paths for the target element's GENDF files in the selected
            directory.
        warnings (int): Count of the number of isotopes searched for that do
            not have findable GENDF files in the selected directory.
    """

    warnings = 0
    element_gendf_paths = []

    commands = prepare_ls_commands(args, gendf_dir, element)

    for command, A in commands:
        try:
            ls_output = run_ls_command(command)
            element_gendf_paths.extend(ls_output)
        except subprocess.CalledProcessError as e:
            warn_missing_file(e, element, A)
            warnings += 1

    return element_gendf_paths, warnings

def search_gendf_directory(args, gendf_dir=None, gendf_paths_from_arg=None):
    """
    Iterate through all of the user-selected elements from the argparser to
        find and store all of the corresponding GENDF files that fit elements
        and mass numbers specified. If none of the elements have GENDF files
        that can be found, an Exception will be raised.

    Arguments:
        args (argparse.Namespace): Argparse object that contains user-declared
            parameters.
        gendf_dir (str, optional): Path for the directory containing the
            relevant GENDF file(s). Either this or gendf_paths_from_arg
            must be defined. Defaults to None.
        gendf_paths_from_arg (list of str, optional): List containing only the
            path for the directory containing the relevant GENDF file(s).
            Either this or gendf_dir must be defined. Defaults to None.
    
    Returns:
        all_gendf_paths (list of str): List containing all of the file
            paths to the GENDF files to be analyzed.
    """

    gendf_dir = gendf_dir or gendf_paths_from_arg[0]
    elements = groupr_tools.elements.keys(
    ) if args.element == ['all'] or not args.element else args.element
    missing_elements = 0

    all_gendf_paths = []

    for element in elements:
        element_gendf_paths, warnings = search_files_for_element(args,
                                                                 gendf_dir,
                                                                 element)

        if warnings == len(prepare_ls_commands(args, gendf_dir, element)):
            logger.error(f'No GENDF files found for {element}.')
            missing_elements += 1

        all_gendf_paths.extend(element_gendf_paths)

    if missing_elements == len(elements):
        raise Exception(
            'No GENDF files found for any of the selected elements.'
        )
    else:
        logger.info(
            f'All files to be processed: {", ".join(all_gendf_paths)}'
        )

    return all_gendf_paths

def iterate_MTs(MTs, file_obj, mt_dict, pKZA):
    """
    Iterate through all of the MTs present in a given GENDF file to extract
        the necessary data to be able to run ALARA.
    
    Arguments:
        MTs (list of int): List of reaction types present in the GENDF file.
        file_obj (ENDFtk.tree.File or None): ENDFtk file object containing the
            contents for a specific material's cross-section data.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv
        pKZA (int): Parent KZA identifier.

    Returns:
        gendf_data (pandas.core.frame.DataFrame): Pandas DataFrame containing
            parent KZA values, daughter KZA values, emitted particles,
            counts of the number of non-zero groups in the Vitamin-J groupwise
            structure, and the cross-section values for those groups.
    """

    cross_sections_by_MT = []
    emitted_particles_list = []
    dKZAs = []
    groups = []

    for MT in MTs:
        sigma_list = tpp.extract_cross_sections(file_obj, MT)
        dKZA = pKZA - mt_dict[str(MT)]['delKZA']
        emitted_particles = mt_dict[str(MT)]['Emitted Particles']
        cross_sections_by_MT.append(sigma_list)
        dKZAs.append(dKZA)
        emitted_particles_list.append(emitted_particles)
        groups.append(len(sigma_list))

    gendf_data = pd.DataFrame({
        'Parent KZA': [pKZA] * len(dKZAs),
        'Daughter KZA': dKZAs,
        'Emitted Particles': emitted_particles_list,
        'Non-Zero Groups' : groups,
        'Cross Sections': cross_sections_by_MT
    })

    return gendf_data


def handle_local_file_input(mt_dict, cumulative_data,
                            gendf_paths, gendf_dir, args):
    """
    Method for extracting and analyzing data from preprocessed GENDF files
        that are saved locally. The data extracted is stored in a Pandas
        DataFrame.
    
    Arguments:
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv,
            along with changes in KZA and emitted_particles.
        cumulative_data (pandas.core.frame.DataFrame): Empty Pandas DataFrame
            in which to cumulatively store data extracted from GENDF files.
        gendf_paths (list of str): List containing all of the file paths to
            the GENDF files to be analyzed.
        gendf_dir (str): Path for the directory containing the relevant GENDF
            file(s).
        args (argparse.Namespace): Argparse object that contains the user
            specified arguments for executing the program.
    
    Returns:
        cumulative_data (pandas.core.frame.DataFrame): Cumulatively filled
            Pandas DataFrame containing all of the data extracted from the
            GENDF file(s).
    """

    if not gendf_paths:
        gendf_paths = args.paths

    if not isinstance(gendf_paths, list):
        gendf_paths = [gendf_paths]

    # If the user inputs a path to a directory containing GENDF files for the
    # local file option, then search for the GENDF files of the element(s)
    if gendf_dir or Path(gendf_paths[0]).is_dir():
        gendf_paths = search_gendf_directory(args, gendf_dir, gendf_paths)

    for gendf_path in gendf_paths:

        pKZA, A, element = tpp.extract_gendf_pkza(gendf_path)
        matb, MTs, file_obj = tpp.extract_endf_specs(gendf_path, 'gendf')

        logger.info(f"GENDF file path: {gendf_path}")
        logger.info(f"Parent KZA (pKZA): {pKZA}")
        logger.info(f'MTs: {MTs}')

        gendf_data = iterate_MTs(MTs, file_obj, mt_dict, pKZA)
        cumulative_data = pd.concat([cumulative_data, gendf_data],
                                    ignore_index= True)

        logger.info(
            f'Finished iterating for {element}-{A}{" "*20}{"-"*49}'
        )

    return cumulative_data

def handle_TENDL_downloads(args, mt_dict):
    """
    Method for downloading ENDF/PENDF files from the TENDL 2017 database,
        using the NJOY GROUPR module to convert these to a group-wise file,
        and then extracting and analyzing the resultant data.
    
    Arguments:
        args (argparse.Namespace): Argparse object that contains the user
            specified arguments for executing the program.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv,
                    along with changes in KZA and emitted_particles.
    
    Returns:
        gendf_paths (list of str): List containing all of the file paths to
            the GENDF files that were processed from ENDF/PENDF files by NJOY.
        gendf_dir (str): Path for the directory containing the GENDF file(s).
        
    """

    elements = groupr_tools.elements.keys(
    ) if args.element == ['all'] or not args.element else args.element

    A_vals = args.mass_number

    gendf_paths = []

    for element in elements:
        if A_vals == ['all'] or not A_vals:
            A_vals = asyncio.run(tpp.identify_tendl_isotopes(element))
            logger.info(
                f'All isotopes of {element} in the TENDL database: {A_vals}'
            )

        for A in A_vals:
            endf_path = tpp.download_tendl(element, A, 'endf')
            pendf_path = tpp.download_tendl(element, A, 'pendf')

            material_id, MTs = tpp.extract_endf_specs(endf_path, 'endf')
            
            card_deck = groupr_tools.groupr_input_file_format(material_id,
                                                              MTs,
                                                              element,
                                                              A,
                                                              mt_dict)
            groupr_tools.groupr_input_file_writer(card_deck, MTs)

            gendf_path, gendf_dir = groupr_tools.run_njoy(card_deck, element,
                                                          A, material_id)

            gendf_paths.extend(gendf_path)

            groupr_tools.njoy_file_cleanup()
    
    return gendf_paths, gendf_dir

##############################################################################

def fendl3_2b_retrofit():
    """
    Main method when run as a command line script.
    """

    # Initialize arguments, DataFrame to store data, and load in MT reference
    args = fendl_args()
    cumulative_data = rxd.initialize_dataframe()
    gendf_paths = []
    gendf_dir = None
    mt_dict, dir = rxd.process_mt_table('mt_table.csv')

    # Conditionally download and process files from the TENDL 2017 database
    if not args.paths:
        gendf_paths, gendf_dir = handle_TENDL_downloads(args, mt_dict)

    # Extract and store data from GENDF files
    cumulative_data = handle_local_file_input(mt_dict,
                                              cumulative_data,
                                              gendf_paths = gendf_paths,
                                              gendf_dir = gendf_dir,
                                              args = args)
        
    # Save to CSV
    cumulative_data.to_csv(f'{dir}/gendf_data.csv', index=False)
    logger.info('Saved extracted GENDF data to gendf_data.csv')

##############################################################################

if __name__ == '__main__':
    fendl3_2b_retrofit()