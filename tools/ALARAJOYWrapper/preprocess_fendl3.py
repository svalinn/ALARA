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
        '--temperature', '-t', required=False, nargs=1, default=[293.16]
    )
    return parser.parse_args()

def write_dsv(dsv_path, cumulative_data):
    """
    Write out a space-delimited DSV file from the list of dictionaries,
        dsv_path, produced by iterating through each reaction of each isotope
        to be processed. Each row in the resultant DSV file is ordered as such:

            pKZA dKZA emitted_particles non_zero_groups xs_1 xs_2 ... xs_n

        Each row can have different lengths, as only non-zero cross-sections
        are written out. The file is sorted by ascending parent KZA value.

    Arguments:
        dsv_path (str): Filepath for the DSV file to be written.
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

    dir = njt.set_directory()
    search_dir = args().fendlFileDir[0] if args().fendlFileDir else dir
    
    TAPE20 = 'tape20'

    mt_dict = rxd.process_mt_data(rxd.load_mt_table(f'{dir}/mt_table.csv'))

    cumulative_data = []
    for isotope, file_properties in tp.search_for_files(search_dir).items():
        element = file_properties['Element']
        A = file_properties['Mass Number']
#        endf_path, pendf_path = file_properties['File Paths']
        endf_path = file_properties['TENDL File Path']
        Path(TAPE20).write_bytes(endf_path.read_bytes())
#        Path(TAPE21).write_bytes(pendf_path.read_bytes())

        material_id, MTs, endftk_file_obj = tp.extract_endf_specs(TAPE20)
        MTs = set(MTs).intersection(mt_dict.keys())
        njoy_input = njt.fill_input_template(
            material_id, MTs, element, A, mt_dict,
            temperature=args().temperature[0]
            )
        njt.write_njoy_input_file(njoy_input)
        gendf_path, njoy_error = njt.run_njoy(
            element, A, material_id
        )

        if gendf_path:
            pKZA = tp.extract_gendf_pkza(gendf_path)
            # Extract MT values again from GENDF file as there may be some
            # difference from the original MT values in the ENDF/PENDF files
            material_id, MTs, endftk_file_obj = tp.extract_endf_specs(
                gendf_path
            )
            gendf_data = tp.iterate_MTs(MTs, endftk_file_obj, mt_dict, pKZA)
            cumulative_data.extend(gendf_data)
            njt.cleanup_njoy_files()
            print(f'Finished processing {element}{A}')
        else:
            warnings.warn(
                f'''Failed to convert {element}{A}.
                NJOY error message: {njoy_error}'''
            )

    dsv_path = dir + '/cumulative_gendf_data.dsv'
    write_dsv(dsv_path, cumulative_data)
    print(dsv_path)

if __name__ == '__main__':
    main()