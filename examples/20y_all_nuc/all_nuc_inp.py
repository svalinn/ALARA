import argparse
import yaml
import string

# This script produces the ALARA input file 20y_all_nuc.inp in this directory, with a different element library and some formatting
# differences that may need to be addressed manually

def read_nuclib(nuclib, old_inp_name):
    nuclib_lines = open(nuclib, 'r').readlines()  
    return nuclib_lines

def make_volume_block(nuclib_lines, volume):
    vol_list = []
    load_list = []
    mix_list = []
    for line in nuclib_lines:
        line = line.strip().split()
        nuc = line[0]
        if ':' in nuc:
            vol_list.append(f'\t {volume}\t{nuc}\n')
            load_list.append(f'\t{nuc}\t mix_{nuc}\n')
            mix_list.append(f'mixture mix_{nuc}\n\t element {nuc} 1 1.0 \nend \n')

    return vol_list, load_list, mix_list  

def edit_template(vol_list, load_list, mix_list, old_inp_name, new_inp_name):
    # Uses an existing ALARA input file (in this case ../singleElement_20y_inp/alara_inp_fe_20y) and appends all lines
    # starting from the material_lib line to a new input file.
    blocks = ['volume', 'mat_loading', 'mixture']
    line_lists = [vol_list, load_list, mix_list]
    new_lines = ""
    new_lines += 'geometry rectangular \n \n'
    
    for block_id, block in enumerate(line_lists):
        if block_id != 2:
            new_lines += f'{blocks[block_id]}\n'
        for line in line_lists[block_id]:      
            new_lines += line
        if block_id != 2:    
            new_lines += 'end \n' 

    new_inp = open(new_inp_name, 'w+')
    old_inp = open(old_inp_name, 'r+')

    #Prepare to use old ALARA input file as template onto which new input lines are added
    for old_line in old_inp:
        if old_line.strip().startswith('material_lib'):
            break
        else:
            old_line = ""
    old_inp.seek(0)
    old_inp.write('${placeholder}')
    
    new_inp.write(new_lines)
    template = string.Template(old_inp.read())        
    new_out = template.substitute(placeholder = new_lines)
    new_inp.write(new_out)

#Define all inputs and execute functions:
#---------------------------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--yaml', default = "all_nuc_inp.yaml", help="Path (str) to YAML containing inputs")
    args = parser.parse_args()
    return args

def read_yaml(args):
    with open(args.yaml, 'r') as yaml_file:
        params = yaml.safe_load(yaml_file)
    return params

def main():
    args = parse_args()
    params = read_yaml(args)

    nuclib = params['nuclib']
    old_inp_name = params['old_inp_name']
    new_inp_name = params['new_inp_name']
    volume = params['volume']

    nuclib_lines = read_nuclib(nuclib, old_inp_name)
    vol_list, load_list, mix_list = make_volume_block(nuclib_lines, volume)

    edit_template(vol_list, load_list, mix_list, old_inp_name, new_inp_name)

if __name__ == '__main__':
    main()    