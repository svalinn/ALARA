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

    material_id, MTs = tp.extract_endf_specs(endf_path)
    njoy_input = groupr_tools.fill_input_template(material_id, MTs, 'Fe', 56, mt_dict)
    groupr_tools.write_njoy_input_file(njoy_input)
    groupr_tools.run_njoy('Fe', 56, material_id)

if __name__ == '__main__':
    main()