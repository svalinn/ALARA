# Import packages
import reaction_data as rxd
import tendl_processing as tp
import njoy_tools as njt
import argparse
import warnings
from pathlib import Path
from itertools import chain
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
        # Temperature for NJOY run [Kelvin]
        '--temperature', '-t', required=False, nargs=1, default=[293.16]
    )
    return parser.parse_args()

def amalgamate_by(nuc_type, row_dicts):
    """
    Reorganize a list of reaction dictionaries into a dictionary keyed by
        either parents or daughters, each containing all the reactions that
        either start from the same parent or produce the same daughter,
        respectively.
    
    Arguments:
        nuc_type (str): Amalgamation key, either Parent or Daughter.
        row_dicts (list of dicts): List of containing separate dictionaries
            for each activation reaction.
    Returns:
        amalgam (collections.defaultdict): Dictionary keyed by either parents
            or daughters, each containing nested dictionaries of all reactions
            stemming from or leading to their respective key nuclide.
    """

    amalgam = defaultdict(list)
    for rxn in row_dicts:
        amalgam[rxn[f'{nuc_type.capitalize()} KZA']].append(rxn)

    return amalgam

def build_parent_daughter_struct(cumulative_data):
    by_parent = defaultdict(lambda: defaultdict(dict))

    for rxn in cumulative_data:
        parent = rxn['Parent KZA']
        daughter = rxn['Daughter KZA']
        mt   = rxn['MT']

        # MT is an integer; this creates an entry safely
        by_parent[parent][daughter][mt] = {
            'Emitted Particles': rxn['Emitted Particles'],
            'Non-Zero Groups' : rxn['Non-Zero Groups'],
            'Cross Sections': rxn['Cross Sections']
        }
        
    return by_parent

def remove_gas_daughters(cumulative_data, mt_dict):
    """
    Filter reactions from the list of all reaction row dictionaries to remove
        all reactions resulting in a light gas daughter for each parent
        nuclide, given that parent includes any gas total production
        "reactions", corresponding to MT=203-207. Otherwise, if no gas totals
        are present, preserve all reactions because there is no double
        counting.
        
        Optional method to be called within gas_handling().

    Arguments:
        cumulative_data (list of dicts): List containing separate dictionaries
            for each reaction contained in all of the TENDL/PENDF files
            processed.

    Returns:
        gas_filtered (list of dicts): List of reactions that satisfy the
            conditions that the daughter nuclide is either heavier than an
            alpha particle (i.e. not a light gas) or that the reaction
            represents any of the MT = 203-207 total gas production reactions
            (as denoted by the 'x' emitted particle).
    """
    
    gas_filtered = []
    by_parent = build_parent_daughter_struct(cumulative_data)
    for parent in by_parent.keys():
        for daughter in by_parent[parent].keys():
            mts = set(by_parent[parent][daughter].keys())
            
            deleted_mts = []
            for mt in mts:
                gas_mts = tp.GAS_IDS['total_mt'].tolist()
                if mt not in gas_mts and any(mt in gas_mts for mt in mts):
                    del by_parent[parent][daughter][mt]
                    deleted_mts.append(mt)

            for mt in (mts - set(deleted_mts)):
                gas_filtered.append(tp.format_rxn_row(
                    mt_dict = mt_dict,
                    MT=mt,
                    pKZA=parent,
                    dKZA=daughter,
                    sigma_list=by_parent[parent][daughter][mt]['Cross Sections']
                ))

    return gas_filtered

def gas_handling(gas_method, cumulative_data, mt_dict):
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
        cumulative_data (list of dicts): List containing separate dictionaries
            for each reaction contained in all of the TENDL/PENDF files
            processed.

    Returns:
        gas_filtered (list of dicts): List of reactions that satisfy the
            selected gas handling method.
    """

    if gas_method == 'r':
        return remove_gas_daughters(cumulative_data, mt_dict)

    # Pathway for subtraction method to be developed in a separate PR
    # to close #186
    # if gas_method == 's':
        # return subtract_gas_from_totals(rxn)

def write_dsv(dsv_path, row_dicts):
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
        row_dicts (list of dicts): List containing separate dictionaries
            for each reaction to be written out to the DSV.
    
    Returns:
        None 
    """

    xs_key = 'Cross Sections'
    join_keys = [k for k in row_dicts[0] if k not in [xs_key]]
    # Sort list of reaction dictionaries by ascending parent KZAs
    parent_label = join_keys[0]
    row_dicts.sort(key=lambda rxn: rxn[parent_label])

    with open(dsv_path, 'w') as dsv_file:
        # Write header line with total groups for Vitamin-J
        vitamin_J_energy_groups = 175
        dsv_file.write(str(vitamin_J_energy_groups) + '\n')

        for reaction in row_dicts:
            dsv_row = ' '.join(str(reaction[key]) for key in join_keys)
            dsv_row += ' ' + ' '.join(str(xs) for xs in reaction[xs_key])
            dsv_row += '\n'
            dsv_file.write(dsv_row)
        # End of File (EOF) signifier to be read by ALARAJOY
        dsv_file.write(str(-1))

def main():
    """
    Main method when run as a command line script.
    """

    # Set constants
    TAPE20 = 'tape20'

    dir = njt.set_directory()
    search_dir = (
        Path(args().fendlFileDir[0]) if args().fendlFileDir else dir
        )
    temperature = args().temperature[0]

    TAPE20 = Path('tape20')

    mt_dict = rxd.process_mt_data(rxd.load_mt_table(dir / 'mt_table.csv'))

    cumulative_data = []
    for isotope, file_properties in tp.search_for_files(search_dir).items():
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
        gas_MTs = set(pendf_MTs) & set(tp.GAS_IDS['total_mt'])
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
                gendf_data = tp.iterate_MTs(
                    MTs, endftk_file_obj, mt_dict, pKZA
                )
                cumulative_data.extend(gendf_data)
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
    gas_filtered = gas_handling(args().gas_handling[0], cumulative_data, mt_dict)
    
    dsv_path = dir / 'cumulative_gendf_data.dsv'
    write_dsv(dsv_path, gas_filtered)
    print(dsv_path)

if __name__ == '__main__':
    main()