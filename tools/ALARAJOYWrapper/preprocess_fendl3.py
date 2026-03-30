# Import packages
import reaction_data as rxd
import tendl_processing as tp
import njoy_tools as njt
import numpy as np
import argparse
import warnings
from pathlib import Path
from collections import defaultdict

ISOMERIC_STATES = 'mnopqrstuvwxyz'

def args():
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
    return parser.parse_args()

def calculate_pKZA(element, A):
    """
    Construct the target (parent) nuclide's KZA value. KZA values are defined
        as ZZAAAM, where ZZ is the nuclide's atomic number, AAA is the mass
        number, and M is the isomeric state (0 if ground state).

    Arguments:
        element (str): Chemical symbol of the target nuclide.
        A (int or str): Mass number of the nuclide, potentially including an
            isomeric tag, such as "m" for the first excited state, "n" for the
            second, etc. (Note: TENDL data should only include up to a maximum
            of the second excited state).

    Returns:
        pKZA (int): KZA value of the target nuclide.
    """

    Z = njt.elements[element]
    A = str(A).lower()

    # Metastable states classified by TENDL as m = 1, n = 2, etc.
    # (Generally expecting only m, occasionally n, but physically,
    # values could go higher, so isomeric_states goes up to z = 14)
    M = 0
    isomer_tag = next(
        (tag for tag in ISOMERIC_STATES if tag in A), None
    )
    if isomer_tag:
        M = ISOMERIC_STATES.find(isomer_tag) + 1
    
    if M > 2:
        warnings.warn(
            f'Isomeric state greater than 2. Unexpected case for TENDL2017.',
            UserWarning
        )

    A = int(A.split(isomer_tag)[0])
    return (Z * 1000 + A) * 10 + M

def interpret_KZA(kza):
    """
    Infer the chemical symbol and mass number from a KZA (ZZAAAM) number.

    Arguments:
        kza (int): Unique ZZAAAM for a given nuclide.

    Returns:
        element (str): Chemical symbol of the target nuclide.
        A (str or int): Mass number for selected isotope.
            If the target is a metastable isomer, "m" or "n" is written after 
            the mass number, corresponding to the first or second metastable
            states.
    """

    kza = str(kza)
    A = kza[-4:-1]
    Z = kza[:kza.find(A)].zfill(2)
    element = list(njt.elements.keys())[int(Z) - 1]
    M = int(kza[-1])
    if M > 0:
        A += ISOMERIC_STATES[M - 1]

    return element, A

def process_pendf(
    njoy_prep_input, material_id, MTs, pKZA, mt_dict, temperature, tendl_path
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
    """

    element, A = interpret_KZA(pKZA)
    njoy_error = ''
    njoy_input = njt.fill_input_template(
        njoy_prep_input, material_id, MTs, element, A, mt_dict, temperature
    )
    njt.write_njoy_input_file(njoy_input)
    pendf_path, njoy_error = njt.run_njoy(element, A, material_id, 'PENDF')

    _, pendf_MTs = tp.extract_endf_specs(pendf_path)
    MTs |= set(pendf_MTs).intersection(set(rxd.GAS_DF['total_mt']))
    isomer_dict = tp.determine_all_excitations(tendl_path, MTs, pKZA, mt_dict)

    return MTs, isomer_dict, njoy_error

def process_gendf(
    njoy_groupr_input, material_id, MTs, mt_dict,
    temperature, pKZA, isomer_dict, all_rxns, eaf_nucs
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

    Returns:
        all_rxns (collections.defaultdict): Updated dictionary for all
            reaction pathways for the given parent and its MTs.
    """

    element, A = interpret_KZA(pKZA)
    groupr_input = njt.fill_input_template(
        njoy_groupr_input, material_id, MTs, element,
        A, mt_dict, temperature, pKZA, isomer_dict
    )
    njt.write_njoy_input_file(groupr_input)
    gendf_path, njoy_error = njt.run_njoy(
        element, A, material_id, 'GENDF'
    )

    if gendf_path:
        # Extract MT values again from GENDF file as there may be some
        # difference from the original MT values in the ENDF/PENDF files
        xs_line_dict, gendf_MTs = tp.extract_gendf_data(gendf_path)
        if MTs != gendf_MTs:
            diffs = sorted(MTs - gendf_MTs)
            warnings.warn(
                f'GENDF file missing MTs {diffs} present in the ' \
                'original TENDL file.'
            )
        if gendf_MTs:
            all_rxns = tp.iterate_MTs(
                gendf_MTs, mt_dict, xs_line_dict, pKZA, 
                all_rxns, eaf_nucs, isomer_dict, gendf_path
            )
            print(f'Finished processing {element}{A}')

        else:
            warnings.warn(
                f'''The requested file (MF3) is not present in the
                ENDF file tree for {element}{A}'''
            )
            with open('mf_fail.log', 'a') as fail:
                fail.write(f'{element}{A} \n')

    else:
        warnings.warn(
            f'''Failed to convert {element}{A}.
            NJOY error message: {njoy_error}'''
        )

    return all_rxns

def remove_gas_daughters(all_rxns, gas_tuples):
    """
    Remove reactions from the dictionary that produce a light gas daughter
        (i.e. a nuclide lighter than an alpha particle) whose total gas
        production is otherwise accounted for by the MT = 203-207 "reactions"
        to avoid double-counting.

        Optional method to be called within gas_handling().

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
        gas_tuples (list of tuples): Pairs total gas production MT values with
            their respective gas symbols of the form [(gas, MT), ...].
 
    Returns:
        all_rxns (collections.defaultdict): Modified version of all_rxns with
            double-counted gas-producing reactions left out.
    """

    for parent in all_rxns:
        for gKZA, gMT in gas_tuples:
            if gKZA in all_rxns[parent] and gMT in all_rxns[parent][gKZA]:
                all_rxns[parent][gKZA] = {gMT: all_rxns[parent][gKZA][gMT]}

    return all_rxns

def subtract_gas_from_totals(all_rxns, gas_tuples):
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
            if gKZA in all_rxns[parent] and gMT in all_rxns[parent][gKZA]:
                for gRxn in all_rxns[parent][gKZA]:
                    if gRxn != gMT:
                        all_rxns[parent][gKZA][gMT]['xsections'] -= (
                            all_rxns[parent][gKZA][gRxn]['xsections']
                        )

    return all_rxns

def combine_daughter_pathways(gas_filtered):
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
                'emitted'    :                                 set(),
                'xsections'  :  np.zeros(tp.VITAMIN_J_ENERGY_GROUPS)
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

def write_dsv(dsv_path, all_rxns):
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
        dsv.write(str(tp.VITAMIN_J_ENERGY_GROUPS) + '\n')
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

    TAPE20 = Path('tape20')

    dir = njt.set_directory()
    search_dir = (
        Path(args().fendlFileDir[0]) if args().fendlFileDir else dir
        )
    temperature = args().temperature[0]

    mt_dict = rxd.process_mt_data(rxd.load_mt_table(dir / 'mt_table.csv'))
    eaf_nucs = rxd.find_eaf_ref_data(Path(args().decay_lib[0]))
    all_rxns = defaultdict(lambda: defaultdict(dict))

    for file_properties in tp.search_for_files(search_dir):
        element = file_properties['Element']
        A = file_properties['Mass Number']
        pKZA = calculate_pKZA(element, A)
        endf_path = file_properties['TENDL File Path']
        TAPE20.write_bytes(endf_path.read_bytes())

        material_id, MTs, endftk_file_obj = tp.extract_endf_specs(TAPE20)
        endf6_MTs = set(mt_dict.keys())
        if MTs != endf6_MTs:
            invalid_MTs = sorted(MTs - endf6_MTs)
            warnings.warn(
                f'Invalid MTs in provided TENDL file for' \
                f'{element}-{A}: {invalid_MTs}'
            )
        MTs = set(MTs).intersection(endf6_MTs)

        MTs, isomer_dict, njoy_prep_error = process_pendf(
            njt.njoy_prep_input, material_id, MTs,
            pKZA, mt_dict, temperature, TAPE20
        )

        if not njoy_prep_error:
            all_rxns = process_gendf(
                njt.groupr_input, material_id, MTs, mt_dict,
                temperature, pKZA, isomer_dict, all_rxns, eaf_nucs 
            )

        else:
            warnings.warn(
                f'''PENDF preparation failed for {element}{A}.
                NJOY error message: {njoy_prep_error}'''
            )

        njt.cleanup_njoy_files(element, A)

    # Handle gas total production cross-sections, per user specifications
    gas_filtered = subtract_gas_from_totals(all_rxns)

    if args().amalgamate:
        gas_filtered = combine_daughter_pathways(gas_filtered)

    dsv_path = dir / 'cumulative_gendf_data.dsv'
    write_dsv(dsv_path, gas_filtered)
    print(f'Reaction data saved to: {dsv_path}')

if __name__ == '__main__':
    main()