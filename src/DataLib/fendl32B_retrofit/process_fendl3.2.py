# Import packages
import reaction_data as rxd
import tendl_preprocessing as tpp
import groupr_tools

def main():
    """
    Main method when run as a command line script.
    """

    mt_dict = rxd.process_mt_data(rxd.load_mt_table('mt_table.csv'))

    endf_path = 'files_for_tests/endf_test1.tendl'
    pendf_path = 'files_for_tests/pendf_test1.pendf'

    material_id, MTs = tpp.extract_endf_specs(endf_path)
    njoy_input = groupr_tools.fill_input_template(material_id, MTs, 'Fe', 56, mt_dict)
    groupr_tools.write_njoy_input_file(njoy_input)

if __name__ == '__main__':
    main()