# Import packages
import reaction_data as rxd
import tendl_processing as tp
import groupr_tools
import file_handling as fh
from pandas import concat

def main():
    """
    Main method when run as a command line script.
    """

    TAPE20 = 'tape20'
    TAPE21 = 'tape21'

    mt_dict = rxd.process_mt_data(rxd.load_mt_table('mt_table.csv'))
    cumulative_data = tp.cumulative_data

    for isotope, file_properties in fh.search_for_files().items():
        element = file_properties['Element']
        A = file_properties['Mass Number']
        endf_path, pendf_path = file_properties['File Paths']
        endf_path.rename(TAPE20)
        pendf_path.rename(TAPE21)

        material_id, MTs, endftk_file_obj = tp.extract_endf_specs(TAPE20)
        MTs = set(MTs).intersection(mt_dict.keys())
        njoy_input = groupr_tools.fill_input_template(material_id, MTs,
                                                      element, A, mt_dict)
        groupr_tools.write_njoy_input_file(njoy_input)
        gendf_path = groupr_tools.run_njoy(element, A, material_id)

        pKZA = tp.extract_gendf_pkza(gendf_path)
        # Extract MT values again from GENDF file as there may be some
        # difference from the original MT values in the ENDF/PENDF files
        material_id, MTs, endftk_file_obj = tp.extract_endf_specs(gendf_path)
        gendf_data = tp.iterate_MTs(MTs, endftk_file_obj, mt_dict, pKZA)
        cumulative_data = concat([cumulative_data, gendf_data],
                                 ignore_index=True)
        groupr_tools.cleanup_njoy_files()

    cumulative_data.to_csv('cumulative_gendf_data.csv')

if __name__ == '__main__':
    main()