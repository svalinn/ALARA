# Import packages
import reaction_data as rxd
import tendl_processing as tp
import njoy_tools as njt
import argparse
import warnings
from pathlib import Path

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

def remove_gas_daughters(rxn):
    '''
    Remove all reactions resulting in a light gas daughter. Optional
        method to be called ahead of data writing to file in write_dsv().

    Arguments:
        rxn (dict): Dictionary containing a row of reaction data for each
            reaction pathway, containing the groupwise cross-section data.

    Returns:
        is_not_gas_daug (bool): Boolean condition to determine whether or not 
            a reaction will be written out to the DSV based on if the daughter
            is either heavier than an alpha particle (i.e. not a light gas) or
            if the reaction represents any of the MT = 203-207 total gas
            production reactions (as denoted by the 'x' emitted particle).
    '''

    alpha_kza = 20040 # Heaviest gas in interest alpha-particle
    return rxn['Daughter KZA'] > alpha_kza or rxn['Emitted Particles'] == 'x'

def gas_handling(gas_method, rxn):
    '''
    Set handling method for gas production total cross-sections for any given
        reaction to determine whether it will be written out to the DSV or 
        not. Either remove_gas_daughters() or subtract_gas_from_totals()
        required for gas total handling methods. If neigther is chosen, an
        error will be raised.

    Arguments:
        gas_method (str): Choice of method for handling gas production total
            cross-sections. Either 'r' (remove) or 's' (subtract). See
            ALARAJOYWrapper/README.md for futher details on these methods.
        rxn (dict): Dictionary containing a row of reaction data for each
            reaction pathway, containing the groupwise cross-section data.

    Returns:
        write_out (bool): Boolean condition for whether a particular reaction
            will be written out to the DSV, based on its matching of the given
            gas handling method.
    '''

    if gas_method == 'r':
        return remove_gas_daughters(rxn)

    # Pathway for subtraction method to be developed in a separate PR
    # to close #186
    # if gas_method == 's':
        # return subtract_gas_from_totals(rxn)

def write_dsv(dsv_path, cumulative_data, gas_method):
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
        cumulative_data (list of dicts): List containing separate dictionaries
            for each reaction contained in all of the TENDL/PENDF files
            processed.
    Returns:
        None 
    """

    xs_key = 'Cross Sections'
    join_keys = list(cumulative_data[0].keys())
    join_keys.remove(xs_key)
    # Sort list of reaction dictionaries by ascending parent KZAs
    parent_label = join_keys[0]
    cumulative_data.sort(key=lambda rxn: rxn[parent_label])

    with open(dsv_path, 'w') as dsv_file:
        # Write header line with total groups for Vitamin-J
        vitamin_J_energy_groups = 175
        dsv_file.write(str(vitamin_J_energy_groups) + '\n')

        for reaction in cumulative_data:
            if gas_handling(gas_method, reaction):
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
    GAS_MT_MIN = 203 # Lowest MT number in range of gas production totals
    GAS_MT_MAX = 207 # Highest MT number in range of gas production totals


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
        gas_MTs = set(pendf_MTs) & set(range(GAS_MT_MIN, GAS_MT_MAX + 1))
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

    #remove_gas_daughters(cumulative_data)
    dsv_path = dir / 'cumulative_gendf_data.dsv'
    write_dsv(dsv_path, cumulative_data, args().gas_handling[0])
    print(dsv_path)

if __name__ == '__main__':
    main()