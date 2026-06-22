# Import packages
import reaction_data as rxd
import tendl_processing as tp
import njoy_tools as njt
import numpy as np
import argparse
import warnings
import logging
from pathlib import Path
from collections import defaultdict
from subprocess import TimeoutExpired

def make_argparser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--decay_lib', '-d', required=True, nargs=1,
        help=('''
            Required argument to direct ALARAJOYWrapper to an EAF decay
                library or directory containing EAF decay files for
                individual nuclides. Necessary for cross-referencing short-
                lived isomeric daughters against known half-life data.
              
                Note: If using --decay_lib to direct to a directory of EAF
                decay files, all files must have the extension ".dat".
        ''')
    )
    parser.add_argument(
        '--fendlFileDir', '-f', required=False, nargs=1,
        help=('''
            Optional argument to direct ALARAJOYWrapper to the directory
                containing the TENDL files to be processed. If left blank,
                --fendlFileDir will default to the current working directory.
        ''')
              
    )
    parser.add_argument(
        '--amalgamate', '-a', action='store_true',
        help=('''
            Optional argument to amalgamate all like-daughters of a given
                parent into a single row of the resultant DSV file file by
                adding the groupwise cross-section data for all reaction
                pathways that produce said daughter from each parent.
        ''')
    )
    parser.add_argument(
        # Temperature for NJOY run [Kelvin]
        '--temperature', '-t', required=False, nargs=1, default=[293.16]
    )
    parser.add_argument(
        '--redirect_warnings', '-r', action='store_true',
        help=('''
            Optional argument to redirect any non-fatal warnings that may
                arise to a log file.
        ''')
    )
    parser.add_argument(
        '--group_structure', '-g', required=False, nargs=1, default=['17'],
        help=('''
            Specification for group structure in which to convert TENDL cross-
                sections. See njoy_tools.set_group_structure() for specific
                details for acceptable forms in which to supply this argument.
        ''')
    )
    return parser

def configure_logging(redirect_warnings=False):
    """
    Configure a logger to redirect output messages away from the terminal.

    Arguments:
        redirect_warnings (bool, optional): Option to redirect warning
            messages to a separate 'warnings.log' file to avoid cluttering
            the terminal with non-fatal warnings.

    Returns:
        None
    """

    logger = logging.getLogger()
    logger.setLevel(logging.INFO)

    formatter = logging.Formatter(
        '%(asctime)s - %(levelname)s: \n\t%(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )

    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)

    if redirect_warnings:
        console_handler.setLevel(logging.INFO)
        console_handler.addFilter(lambda record: record.levelno < logging.WARNING)

        warning_log = Path('warnings.log')
        warning_log.unlink(missing_ok=True)

        file_handler = logging.FileHandler(warning_log)
        file_handler.setLevel(logging.WARNING)
        file_handler.setFormatter(formatter)

        logger.addHandler(file_handler)
        warnings.simplefilter('always')
        logging.captureWarnings(True)

    logger.addHandler(console_handler)

def process_pendf(
    material_id, MTs, pKZA, mt_dict, temperature,
    tendl_path, tendl_dir, unresr_err_cases
):
    """
    Prepare and run initial NJOY run with MODER, RECONR, BROADR, UNRESR, and
        GASPR modules to prodece the requisite PENDF file for a subsequent
        NJOY run with GROUPR. Concurrently, update the set of MTs with total
        gas production values, store the KZA of the target nuclide, and create
        a dictionary of all isomeric pathways for each MT.

    Arguments:
        njoy_prep_input (string.Template): Unfilled Template object for the
            preparatory NJOY input file.
        material_id (int): Unique material identifier, defined by the ENDF-6
            Formats Manual
            (https://www.oecd-nea.org/dbdata/data/manual-endf/endf102.pdf).
        MTs (set): Set of all MTs from the original TENDL file.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv.
        temperature (float): Temperature at which to run NJOY modules.
        tendl_path (pathlib._local.PosixPath): Path to the original,
            unmodified TENDL file.
        tendl_dir (pathlib._local.PosixPath): Path to the directory in which
            the original TENDL nuclide files are contained.
        unresr_err_cases (list of str): List of all nuclides that required an
            increase in the fractional error tolerance for UNRESR.

    Returns:
        MTs (set): Updated set of all reaction types shared between the
            original TENDL file and the processed PENDF file, specifically
            including total gas production reactions.
        isomer_dict (collections.defaultdict): Dictionary keyed by reaction
            type (MT), with each MT containing a subdictionary of the MF from
            which the isomeric pathways are extracted. At the lowest MT/MF
            level has a list of all isomeric states of possible daughter
            nuclides for which there are cross-section data in the original
            TENDL file.
        njoy_error (str): Error message from the NJOY run. Empty string if run
            is successful.
        unresr_err_cases (list of str): Updated list of nuclides that required
            an increase in the fractional error tolerance for UNRESR, if the
            given nuclide was so.
    """

    element, A = tp.interpret_KZA(pKZA)
    njoy_error = ''

    # Run MODER, RECONR, BROADR, UNRESR
    # If fractional error tolerance to low for RECONR, can cause NJOY to time
    # out. If this occurs, increase error tolerance until NJOY can run
    # successfully.
    err = 0.001
    max_err = 0.02
    timeouts = [60, 300] # s
    success = False
    while err <= max_err and not success:
        for timeout in timeouts:
            njoy_prep_input = njt.fill_input_template(
                njt.njoy_prep_input, material_id, MTs,
                element, A, mt_dict, temperature, err=err
            )
            njt.write_njoy_input_file(njoy_prep_input)
            pendf_path, prep_error, njoy_out = njt.run_njoy(
                element, A, material_id, 'PENDF', tendl_dir, timeout=timeout
            )

            if not isinstance(prep_error, TimeoutExpired):
                success = True
                break
    
        if success:
            break

        err += 0.001

        print(
            f'NJOY timed out for {element}{A}. Increasing RECONR fractional '\
            f'reconstruction tolerance to {err:.3f}.'
        )

        if err > max_err:
            warnings.warn(
                f'NJOY repeatedly timed out for {element}{A} up to err='\
                f'{max_err:.3f}. Skipping file.'
            )
            return set(), dict(), prep_error

    # If UNRESR fails, run GASPR with PENDF output of BROADR with warning
    unresr_error_flag = '***error in rdunf2***'
    if prep_error and unresr_error_flag in njoy_out:
        warnings.warn(
            'UNRESR failed to produce cross-sections in the unresolved '\
            f'energy range for {element}-{A}:\n'\
            f'{njoy_out[njoy_out.find(unresr_error_flag):]}Skipping to GASPR.'
        )
        unresr_err_cases.append(f'{element}-{A}')
        gaspr_input = njt.fill_input_template(
            njt.gaspr_input, material_id, MTs, element, A,
            mt_dict, temperature, unresr_fail=True
        )
        njt.write_njoy_input_file(gaspr_input)
        pendf_path, gaspr_error, _ = njt.run_njoy(
            element, A, material_id, 'PENDF', tendl_dir
        )
        njoy_error += gaspr_error
    else:
        njoy_error += prep_error

    _, pendf_MTs = tp.extract_endf_specs(pendf_path)
    MTs |= pendf_MTs.intersection(set(rxd.GAS_DF['total_mt']))
    isomer_dict = tp.determine_all_excitations(tendl_path, MTs, pKZA, mt_dict)

    return MTs, isomer_dict, njoy_error, unresr_err_cases

def process_gendf(
    njoy_groupr_input, material_id, MTs, mt_dict,
    temperature, pKZA, isomer_dict, all_rxns, eaf_nucs, tendl_dir,
    ign=17, ngn='', egn=''
):
    """
    Prepare and run NJOY run with GROUPR and iteratively extract cross-section
        data for each reaction type, with all excitation pathways to be saved
        in the all_rxns dictionary.

    Arguments:
        njoy_groupr_input (string.Template): Unfilled Template object for the
            GROUPR NJOY input file.
        material_id (int): Unique material identifier, defined by the ENDF-6
            Formats Manual
            (https://www.oecd-nea.org/dbdata/data/manual-endf/endf102.pdf).
        MTs (set): Set of all MTs from the original TENDL file.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv.
        temperature (float): Temperature at which to run NJOY modules.
        pKZA (int): KZA identifier of the target (parent) nuclide.
        isomer_dict (collections.defaultdict): Dictionary keyed by reaction
            type (MT), with each MT containing a subdictionary of the MF from
            which the isomeric pathways are extracted. At the lowest MT/MF
            level has a list of all isomeric states of possible daughter
            nuclides for which there are cross-section data in the original
            TENDL file.
        all_rxns (collections.defaultdict): Hierarchical dictionary keyed by
            parent nuclides to store all reaction data, with structured as:
            {parent:
                {daughter:
                    {MT:
                        {
                            'emitted': (str of emitted particles)
                            'xsections': (array of groupwise XS)
                        }
                    }
                }    
            }
        eaf_nucs (dict): Dictionary keyed by all radionuclides in the EAF
            decay library, with values of their half-lives.
        tendl_dir (pathlib._local.PosixPath): Path to the directory in which
            the original TENDL nuclide files are contained.
        ign (str or int, optional): GROUPR neutron group structure parameter.
            ign = 1 for arbitrary group structures not contained in NJOY's
            built-in list of options. Default value corresponds to ign key for
            the Vitamin-J 175 energy group structure.
            (Defaults to 17)
        ngn (str, optional): Number of groups. Will be an empty string unless
            ign == 1.
            (Defaults to '')
        egn (str): Space-joined string of all energy group bounds in ascending
            order. Will be an empty string unless ign == 1.
            (Defaults to '')

    Returns:
        all_rxns (collections.defaultdict): Updated dictionary for all
            reaction pathways for the given parent and its MTs.
    """

    element, A = tp.interpret_KZA(pKZA)
    groupr_input = njt.fill_input_template(
        njoy_groupr_input, material_id, MTs, element,
        A, mt_dict, temperature, pKZA, isomer_dict,
        ign=ign, ngn=ngn, egn=egn
    )
    njt.write_njoy_input_file(groupr_input)
    gendf_path, njoy_error, _ = njt.run_njoy(
        element, A, material_id, 'GENDF', tendl_dir
    )

    if gendf_path:
        # Extract MT values again from GENDF file as there may be some
        # difference from the original MT values in the ENDF/PENDF files
        non_zero_xs, gendf_MTs, nGroups = tp.extract_gendf_data(gendf_path)

        if MTs != gendf_MTs:
            diffs = sorted(MTs - gendf_MTs)
            warnings.warn(
                f'GENDF file missing MTs {diffs} present in the ' \
                f'original TENDL file for {element}-{A}.'
            )
        if gendf_MTs:
            all_rxns = tp.iterate_MTs(
                gendf_MTs, mt_dict, non_zero_xs, pKZA, 
                all_rxns, eaf_nucs, isomer_dict, nGroups
            )
            print(f'Finished processing {element}-{A}')

        else:
            warnings.warn(
                f'''The requested file (MF3) is not present in the
                ENDF file tree for {element}-{A}'''
            )
            with open('mf_fail.log', 'a') as fail:
                fail.write(f'{element}-{A} \n')

    else:
        warnings.warn(
            f'''Failed to convert {element}-{A}.
            NJOY error message: {njoy_error}'''
        )

    return all_rxns, nGroups

def subtract_gas_from_totals(all_rxns):
    """
    For any reaction that produces a gas daughter, subtract the individual
        cross-sections from the list of total gas production cross sections
        corresponding to MT = 203-207. Optional method to be called within
        gas_handling().

    Arguments:
        all_rxns (collections.defaultdict): Hierarchical dictionary keyed by
            parent nuclides to store all reaction data, with structured as:
            {parent:
                {daughter:
                    {MT:
                        {
                            'emitted': (str of emitted particles)
                            'xsections': (array of groupwise XS)
                        }
                    }
                }    
            }
    
    Returns:
        all_rxns (collections.defaultdict): Modified version of all_rxns with
            each reaction that produces a gas daughter calculating its cross-
            sections through subtraction from gas production totals.

    """

    gas_tuples = list(rxd.GAS_DF[['kza', 'total_mt']].itertuples(index=False))
    for parent in all_rxns:
        for gKZA, gMT in gas_tuples:
            gMT = str(gMT)
            if gKZA in all_rxns[parent] and gMT in all_rxns[parent][gKZA]:
                for gRxn in all_rxns[parent][gKZA]:
                    if gRxn != gMT:
                        all_rxns[parent][gKZA][gMT]['xsections'] -= (
                            all_rxns[parent][gKZA][gRxn]['xsections']
                        )

    return all_rxns

def combine_daughter_pathways(gas_filtered, nGroups):
    """
    Calculate cumulative cross-sections from all reaction pathways that
        produce like daughters.
    
    Arguments:
        gas_filtered (collections.defaultdict): Modified version of all_rxns
            that has already been processed by one of the two gas handling
            methods.
    
    Returns:
        amalgamated (collections.defaultdict): Reorganized version of
            gas_filtered, with combined reaction pathways from a single
            parent to like-daughters, with cumulative cross-sections. 
    """

    for parent in gas_filtered:
        for daughter, rxn_list in gas_filtered[parent].items():
            collapsed = {
                'emitted'    :  set(),
                'xsections'  :  np.zeros(nGroups)
            }

            for rxn in rxn_list.values():
                collapsed['emitted'].add(rxn['emitted'])
                collapsed['xsections'] += rxn['xsections']

            collapsed['emitted'] = ','.join(collapsed['emitted'])
            gas_filtered[parent][daughter] = { -1 : collapsed }

    return gas_filtered

def rxn_to_str(parent, daughter, rxn):
    """
    Generate the list of strings to be written in a single row of the DSV file.

    Arguments:
        parent (int): KZA of the reaction's parent nuclide.
        daughter (int): KZA of the reaction's daughter nuclide.
        rxn (dict): Dictionary for the given reaction containing cross-section
            and emitted particle data.

    Returns:
        dsv_row (str): Joined string of all row data for a single reaction.
    """

    dsv_row = f'{parent} {daughter} {rxn['emitted']} '
    dsv_row += ' '.join(str(xs) for xs in rxn['xsections'])

    return dsv_row

def write_dsv(dsv_path, all_rxns, nGroups):
    """
    Write out a space-delimited DSV file from the list of dictionaries,
        dsv_path, produced by iterating through each reaction of each isotope
        to be processed. Each row in the resultant DSV file is ordered as such:
            pKZA dKZA emitted_particles non_zero_groups xs_1 xs_2 ... xs_n
        Each row can have different lengths, as only non-zero cross-sections
        are written out. The file is sorted by ascending parent KZA value.

    Arguments:
        dsv_path (pathlib._local.PosixPath): Filepath for the DSV file to be
            written.
        all_rxns (collections.defaultdict): Hierarchical dictionary keyed by
            parent nuclides to store all reaction data, with structured as:
            {parent:
                {daughter:
                    {MT:
                        {
                            'emitted': (str of emitted particles)
                            'xsections': (array of groupwise XS)
                        }
                    }
                }    
            }

    Returns:
        None 
    """

    with open(dsv_path, 'w') as dsv:
        dsv.write(str(nGroups) + '\n')
        for parent in sorted(all_rxns):
            for daughter in all_rxns[parent]:
                if parent != daughter:
                    for rxn in all_rxns[parent][daughter].values():
                        if rxn['xsections'].sum() > 0:
                            dsv.write(f'{rxn_to_str(parent,daughter,rxn)}\n')
        # End of File (EOF) signifier to be read by ALARAJOY
        dsv.write(str(-1))

def main():
    """
    Main method when run as a command line script.
    """

    argparser = make_argparser()
    args = argparser.parse_args()

    warnings.formatwarning = (
        lambda msg, cat, fname, lineno, file=None, line=None:
            f"{Path(fname).name}:{lineno}: {cat.__name__}: {msg}\n"
    )
    configure_logging(args.redirect_warnings)

    TAPE20 = Path('tape20')

    dir = njt.set_directory()
    search_dir = (
        Path(args.fendlFileDir[0]) if args.fendlFileDir else dir
    )
    temperature = args.temperature[0]
    ign, ngn, egn, group_name = njt.set_group_structure(args.group_structure)

    mt_dict = rxd.process_mt_data(rxd.load_mt_table(dir / 'mt_table.csv'))
    eaf_nucs = rxd.find_eaf_ref_data(Path(args.decay_lib[0]))
    all_rxns = defaultdict(lambda: defaultdict(dict))

    unresr_err_cases = []
    for file_properties in tp.search_for_files(search_dir):
        element, A, pKZA, endf_path = tuple(file_properties.values())
        TAPE20.write_bytes(endf_path.read_bytes())

        material_id, MTs = tp.extract_endf_specs(TAPE20)
        endf6_MTs = set(mt_dict)
        if len((MTs - rxd.SPEC_MTS) - endf6_MTs) > 0:
            invalid_MTs = sorted((MTs - rxd.SPEC_MTS) - endf6_MTs)
            warnings.warn(
                f'Invalid MTs in provided TENDL file for ' \
                f'{element}{A}: {invalid_MTs}'
            )
        MTs = MTs.intersection(endf6_MTs)

        MTs, isomer_dict, njoy_prep_error, unresr_err_cases = process_pendf(
            material_id, MTs, pKZA, mt_dict, temperature,
            TAPE20, search_dir, unresr_err_cases
        )

        if not njoy_prep_error:
            all_rxns, nGroups = process_gendf(
                njt.groupr_input, material_id, MTs, mt_dict,
                temperature, pKZA, isomer_dict, all_rxns, eaf_nucs,
                search_dir, ign=ign, ngn=ngn, egn=egn
            )

        else:
            warnings.warn(
                f'''PENDF preparation failed for {element}-{A}.
                NJOY error message: {njoy_prep_error}'''
            )

        njt.cleanup_njoy_files(element, A)

    warnings.warn(
        f'A total of {len(unresr_err_cases)} TENDL files required ' \
        'an increase in UNRESR fractional error tolerance for the following' \
        f' nuclides: {unresr_err_cases}'
    )

    # Handle gas total production cross-sections, per user specifications
    gas_filtered = subtract_gas_from_totals(all_rxns)

    if args.amalgamate:
        gas_filtered = combine_daughter_pathways(gas_filtered, nGroups)

    dsv_path = dir / 'cumulative_gendf_data.dsv'
    write_dsv(dsv_path, gas_filtered, nGroups)
    print(
        f'Neutron activation cross-sections converted to {nGroups} groups ' \
        f'according to the {group_name} group structure.'
    )
    print(f'Reaction data saved to: {dsv_path}')

if __name__ == '__main__':
    main()