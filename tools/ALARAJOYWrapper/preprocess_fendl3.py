# Import packages
import reaction_data as rxd
import tendl_processing as tp
import njoy_tools as njt
import numpy as np
import argparse
import warnings
from pathlib import Path
from collections import defaultdict

def args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--fendlFileDir', '-f', required=False, nargs=1
    )
    parser.add_argument(
        '--gas_handling', '-g', required=True, nargs=1
    )
    parser.add_argument(
        '--decay_lib', '-d', required=True, nargs=1
    )
    parser.add_argument(
        '--amalgamate', '-a', action='store_true'
    )
    parser.add_argument(
        # Temperature for NJOY run [Kelvin]
        '--temperature', '-t', required=False, nargs=1, default=[293.16]
    )
    return parser.parse_args()

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
        gas_tuples (list of tuples): Pairs total gas production MT values with
            their respective gas symbols of the form [(gas, MT), ...].
    
    Returns:
        all_rxns (collections.defaultdict): Modified version of all_rxns with
            each reaction that produces a gas daughter calculating its cross-
            sections through subtraction from gas production totals.

    """

    for parent in all_rxns:
        for gKZA, gMT in gas_tuples:
            if gKZA in all_rxns[parent] and gMT in all_rxns[parent][gKZA]:
                for gRxn in all_rxns[parent][gKZA]:
                    if gRxn != gMT:
                        all_rxns[parent][gKZA][gMT]['xsections'] -= (
                            all_rxns[parent][gKZA][gRxn]['xsections']
                        )

    return all_rxns

def gas_handling(gas_method, all_rxns):
    """
    Set handling method for gas production total cross-sections for any given
        reaction to determine whether it will be written out to the DSV or 
        not. Either remove_gas_daughters() or subtract_gas_from_totals()
        required for gas total handling methods. If neigther is chosen, an
        error will be raised.

    Arguments:
        gas_method (str): Choice of method for handling gas production total
            cross-sections. Either 'r' (remove) or 's' (subtract). See
            ALARAJOYWrapper/README.md for futher details on these methods.
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
            double-counted gas-producing reactions left out.
    """

    gas_tuples = list(rxd.GAS_DF[['kza', 'total_mt']].itertuples(index=False))

    if gas_method == 'r':
        return remove_gas_daughters(all_rxns, gas_tuples)

    elif gas_method == 's':
         return subtract_gas_from_totals(all_rxns, gas_tuples)
    
    else:
        raise ValueError(
            'Invalid gas method selection. ' \
            'Must choose either "r" (remove) or "s" (subtract).'
        )

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
    """
    # since there may be entries that are zero before the end of the cross-section
    # we get the index of the last nonzero entry
    last_nonzero = np.nonzero(rxn['xsections'])[-1][-1]

    dsv_row = (
        f'{parent} {daughter} {rxn['emitted']} ' \
        f'{last_nonzero} '
    )
    dsv_row += ' '.join(
        str(xs) for xs
        in rxn['xsections'][:last_nonzero+1]
    )
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
                for rxn in all_rxns[parent][daughter].values():
                    if rxn['xsections'].sum() > 0:
                        dsv.write(rxn_to_str(parent, daughter, rxn) + '\n')
        
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
    radionucs = rxd.find_eaf_radionuclides(Path(args().decay_lib[0]))
    all_rxns = defaultdict(lambda: defaultdict(dict))

    for file_properties in tp.search_for_files(search_dir).values():
        element = file_properties['Element']
        A = file_properties['Mass Number']
        endf_path = file_properties['TENDL File Path']
        TAPE20.write_bytes(endf_path.read_bytes())

        material_id, MTs, endftk_file_obj = tp.extract_endf_specs(TAPE20)
        MTs = set(MTs).intersection(mt_dict.keys())

        # PENDF Preperation and Generation
        njoy_input = njt.fill_input_template(
            njt.njoy_prep_input, material_id,
            MTs, element, A, mt_dict, temperature
            )
        njt.write_njoy_input_file(njoy_input)
        pendf_path, njoy_error = njt.run_njoy(
            element, A, material_id, 'PENDF'
        )
        
        _, pendf_MTs, _ = tp.extract_endf_specs(pendf_path)
        gas_MTs = set(pendf_MTs) & set(rxd.GAS_DF['total_mt'])
        MTs |= {int(gas_MT) for gas_MT in gas_MTs}

        # GENDF Generation
        groupr_input = njt.fill_input_template(
            njt.groupr_input, material_id,
             MTs, element, A, mt_dict, temperature
        )
        njt.write_njoy_input_file(groupr_input)
        gendf_path, njoy_error = njt.run_njoy(
            element, A, material_id, 'GENDF'
        )

        if gendf_path:
            pKZA = tp.extract_gendf_pkza(gendf_path)
            # Extract MT values again from GENDF file as there may be some
            # difference from the original MT values in the ENDF/PENDF files
            material_id, MTs, endftk_file_obj = tp.extract_endf_specs(
                gendf_path
            )

            if MTs and endftk_file_obj:
                all_rxns = tp.iterate_MTs(
                    MTs, endftk_file_obj, mt_dict, pKZA, all_rxns, radionucs
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

        njt.cleanup_njoy_files(element, A)

    # Handle gas total production cross-sections, per user specifications
    gas_filtered = gas_handling(args().gas_handling[0], all_rxns)

    if args().amalgamate:
        gas_filtered = combine_daughter_pathways(gas_filtered)

    dsv_path = dir / 'cumulative_gendf_data.dsv'
    write_dsv(dsv_path, gas_filtered)
    print(f'Reaction data saved to: {dsv_path}')

if __name__ == '__main__':
    main()