import numpy as np
import argparse

def read_out(output_path):
    with open(output_path, "r") as output_file:
        lines = output_file.readlines()
    line_idx = 0
    while not lines[line_idx].startswith("top_schedule 'top_sched':"):
        line_idx += 1  
    return lines[line_idx:]

def read_pulse_histories(lines):    
    '''
    Creates a dictionary with the name of the pulse history as the key, and a dictionary containing the number of pulses
    in all pulsing levels & the delay (in seconds) of all pulsing levels as the value. All pulse histories are stored
    regardless of usage in any schedule item.
    '''
    pulse_dict = {}
    line_idx = 0
    while line_idx < len(lines):
        line = lines[line_idx].strip()
        if line.startswith("pulse_history"):
            num_pulse_line = lines[line_idx+2].strip()
            delay_line = lines[line_idx+3].strip()

            pulse_hist_name = line.split()[1].strip("':")
            num_pulses = num_pulse_line.split(":")[1]
            delays = delay_line.split(":")[1]

            pulse_dict[pulse_hist_name] = {
                "num_pulses_all_levels" : num_pulses,
                "delay_seconds_all_levels" : delays
            }
            line_idx += 4
        else:
            line_idx += 1                                                                                      
    return pulse_dict

def make_sch_sub_dict(sch_line, unit_multipliers):
    sch_sub_dict = {
    "sched_ph_name" : sch_line[3],
    "sched_delay_dur" : unit_multipliers[sch_line[6]] * float(sch_line[5]),
    "sched_delay_unit" : 's'
                   }
    return sch_sub_dict

def make_pe_sub_dict(pe_line, unit_multipliers):
    pe_sub_dict = {
    "pe_dur": unit_multipliers[pe_line[2]] * float(pe_line[1]), 
    "pe_dur_unit": 's', 
    "corr_ph_name": pe_line[4],
    "pe_delay_dur": unit_multipliers[pe_line[7]] * float(pe_line[1]),
    "pe_delay_unit": 's'
                  } 
    return pe_sub_dict

def make_nested_dict(lines):
    '''
    Constructs a hierarchy of dictionaries with a separate level for each additional tab found at the beginning of each line
    in the section of the output with schedule details.
    A sub-dictionary is created for each additional indented level.
    '''
    sch_dict = {}
    last_upper_indent_level = {0: sch_dict}
    line_idx = 0
    unit_multipliers = make_unit_multipliers()
    while not lines[line_idx].startswith("pulse_history:"): # next section of output

        child_level = lines[line_idx].count('\t')
        newline_name = lines[line_idx].lstrip('\t').rstrip('\n')

        if newline_name.strip().split()[0] == 'schedule':
            counter = 1
            last_upper_indent_level[child_level][f'schedule {newline_name.split()[1]}'] = make_sch_sub_dict(newline_name.split(), 
                                                                                                            unit_multipliers)
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level][f'schedule {newline_name.split()[1]}']
            line_idx += 1

        elif newline_name.strip().split()[0] == 'pulse_entry:':
            last_upper_indent_level[child_level][f"{newline_name.split()[0]} num_{counter}_in_sched"] = make_pe_sub_dict(newline_name.split(),
                                                                                                                         unit_multipliers)  
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level][f"{newline_name.split()[0]} num_{counter}_in_sched"]
            counter += 1
            line_idx += 1        
        else: # for line with top schedule
            last_upper_indent_level[child_level][newline_name.split()[1].strip("':")] = {}
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level][newline_name.split()[1].strip("':")]           
            line_idx += 1 
    return sch_dict

def make_unit_multipliers():
    '''
    Defines multipliers to convert all durations and units in schedule/pulse entries to seconds. Changes the corresponding 
    entries in the dictionaries to reflect this change.
    '''
    unit_multipliers = {
    'c' : 60 * 60 * 24 * 365 * 100,
    'y' : 60 * 60 * 24 * 365,
    'w' : 60 * 60 * 24 * 7,
    'd' : 60 * 60 * 24,
    'h' : 60 * 60,
    'm' : 60,
    's' : 1
                     }
    return unit_multipliers

def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", required=True, type=str, help="path to file containing ALARA output")
    arg = parser.parse_args()  
    return arg.f              

def main():
    output_path = parse_arg()
    lines = read_out(output_path)
    unit_multipliers = make_unit_multipliers()

    pulse_dict = read_pulse_histories(lines)
    sch_dict = make_nested_dict(lines)

if __name__ == '__main__':
    main()
