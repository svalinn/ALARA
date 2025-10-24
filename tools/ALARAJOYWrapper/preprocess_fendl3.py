# Import packages
import reaction_data as rxd
import tendl_processing as tp
import njoy_tools as njt
import argparse
import warnings
from pathlib import Path
from pandas import DataFrame
from numpy import array

def args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--fendlFileDir', '-f', required=False, nargs=1
    )
    parser.add_argument(
        # Temperature for NJOY run [Kelvin]
        '--temperature', '-t', required=False, nargs=1, default=[293.16]
    )
    return parser.parse_args()

def main():
    """
    Main method when run as a command line script.
    """

    # Set constants
    TAPE20 = 'tape20'
    GAS_MT_MIN = 203 # Lowest MT number in range of gas production totals
    GAS_MT_MAX = 207 # Highest MT number in range of gas production totals


    dir = njt.set_directory()
    search_dir = args().fendlFileDir[0] if args().fendlFileDir else dir
    temperature = args().temperature[0]

    mt_dict = rxd.process_mt_data(rxd.load_mt_table(f'{dir}/mt_table.csv'))

    cumulative_data = []
    for isotope, file_properties in tp.search_for_files(search_dir).items():
        element = file_properties['Element']
        A = file_properties['Mass Number']
        endf_path = file_properties['TENDL File Path']
        Path(TAPE20).write_bytes(endf_path.read_bytes())

        material_id, MTs, endftk_file_obj = tp.extract_endf_specs(TAPE20)
        MTs = set(MTs).intersection(mt_dict.keys())

        # PENDF Preperation and Generation
        njoy_input = njt.fill_input_template(
            material_id, MTs, element, A, mt_dict, temperature
            )
        njt.write_njoy_input_file(njoy_input)
        pendf_path, njoy_error = njt.run_njoy(
            element, A, material_id, 'PENDF'
        )
        
        _, pendf_MTs, _ = tp.extract_endf_specs(pendf_path)
        pendf_MTs = array(pendf_MTs)
        gas_MTs = pendf_MTs[
            (pendf_MTs >= GAS_MT_MIN) & (pendf_MTs <= GAS_MT_MAX)
            ]
        MTs |= {int(gas_MT) for gas_MT in gas_MTs}

        # GENDF Generation
        groupr_input = njt.fill_input_template(
            material_id, MTs, element, A,
            mt_dict, temperature, run_type='GROUPR'
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
                njt.cleanup_njoy_files()
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

    csv_path = dir + f'/cumulative_gendf_data_{temperature}K.csv'
    DataFrame(cumulative_data).to_csv(csv_path)

    print(csv_path)

if __name__ == '__main__':
    main()