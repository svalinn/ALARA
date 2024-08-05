# Import packages
import reaction_data as rxd
import tendl_processing as tp
import groupr_tools

def main():
    """
    Main method when run as a command line script.
    """

    mt_dict = rxd.process_mt_data(rxd.load_mt_table('mt_table.csv'))

    endf_path = 'tape20'
    pendf_path = 'tape21'

    material_id, MTs, endftk_file_obj = tp.extract_endf_specs(endf_path)
    njoy_input = groupr_tools.fill_input_template(material_id, MTs, 'Fe', 56, mt_dict)
    groupr_tools.write_njoy_input_file(njoy_input)
    gendf_path = groupr_tools.run_njoy('Fe', 56, material_id)

    pKZA = tp.extract_gendf_pkza(gendf_path)
    # Extract MT values again from GENDF file as there may be some difference
    # from the original MT values in the ENDF/PENDF files
    material_id, MTs, endftk_file_obj = tp.extract_endf_specs(gendf_path)
    gendf_data = tp.iterate_MTs(MTs, endftk_file_obj, mt_dict, pKZA)
    gendf_data.to_csv('gendf_data.csv')
    groupr_tools.cleanup_njoy_files()


if __name__ == '__main__':
    main()