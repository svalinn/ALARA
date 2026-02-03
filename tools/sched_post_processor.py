import numpy as np
import argparse

def read_out(output_path):
    with open(output_path, "r") as output_file:
        lines = output_file.readlines()
        for line_idx, line in enumerate(lines):
            # ignore all lines above the one with the top schedule
            if line.startswith("top_schedule 'top_sched':"):
                lines = lines[line_idx:]
    return lines

def read_pulse_histories(out_lines):    
    '''
    Creates a dictionary with the name of the pulse history as the key, and a dictionary containing the number of pulses
    in all pulsing levels & the delay (in seconds) of all pulsing levels as the value. All pulse histories are stored
    regardless of usage in any schedule item.
    '''
    pulse_dict = {}
    for line_idx, line in enumerate(out_lines):
        if line.strip().startswith("pulse_history"):
            pulse_dict[line.split()[1].strip("':")] = {"num_pulses_all_levels": out_lines[line_idx+2].split(":")[1].strip(), 
                                                   "delay_seconds_all_levels": out_lines[line_idx+3].split(":")[1].strip()
                                                      }                                                                                         
    return pulse_dict  

def make_nested_dict(lines):
    '''
    Constructs a hierarchy of dictionaries with a separate level for each additional tab found at the beginning of each line
    in the section of the output with schedule details.
    A sub-dictionary is created for each additional indented level.
    '''
    sch_dict = {}
    last_upper_indent_level = {0: sch_dict}
    for line in lines:
        if line.startswith("pulse_history:"): #next section of output
            break

        newline_name = line.lstrip('\t').rstrip('\n')
        child_level = len(line.rstrip('\n')) - len(newline_name) # find number of tabs in the line

        if newline_name.strip().split()[0] == 'schedule':
            counter = 1
            last_upper_indent_level[child_level]["schedule"] = {
                                                    "sched_name" : line.strip().split()[1],
                                                    "sched_ph_name" : line.strip().split()[3],
                                                    "sched_delay_dur" : line.strip().split()[5],
                                                    "sched_delay_unit" : line.strip().split()[6]
                                                   }
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level]["schedule"]

        elif newline_name.strip().split()[0] == 'pulse_entry:':
            last_upper_indent_level[child_level]['pulse_entry'] = {
                                            "entry_num_in_sched" : counter,
                                            "pe_dur": line.split()[1], 
                                             "pe_dur_unit": line.split()[2], 
                                             "corr_ph_name": line.split()[4],
                                             "pe_delay_dur": line.split()[6],
                                             "pe_delay_unit": line.split()[7]}   
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level]['pulse_entry']
            counter += 1         
        else: # for line with top schedule
            last_upper_indent_level[child_level][newline_name.split()[1].strip("':")] = {}
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level][newline_name.split()[1].strip("':")]           
    return sch_dict

def search_for_match(top_dict, search_str):
    '''
    Takes a nested top dictionary and searches all sub-dictionaries for matches with a provided string.
    '''
    matches = []
    for key, value in top_dict.items():

        if key == search_str:
            matches.append(value)

        elif isinstance(value, dict):
            match_res = search_for_match(value, search_str)
            matches.extend(match_res)

    return matches

def convert_to_s(sch_dict):
    '''
    Defines multipliers to convert all durations and units in schedule/pulse entries to seconds. Changes the corresponding 
    entries in the dictionaries to reflect this change.
    '''
    unit_multiples = {
    'c' : 60 * 60 * 24 * 365 * 100,
    'y' : 60 * 60 * 24 * 365,
    'w' : 60 * 60 * 24 * 7,
    'd' : 60 * 60 * 24,
    'h' : 60 * 60,
    'm' : 60,
    's' : 1
                     }

    search_fields = ["schedule", "pulse_entry"]
    for search_field in search_fields:
        matches = search_for_match(sch_dict, search_field)
        if 'sched_delay_dur' in matches[0]: # if the entry is a schedule entry
            matches[0]['sched_delay_dur'] = float(matches[0]['sched_delay_dur']) * unit_multiples[matches[0]['sched_delay_unit']]
        elif 'pe_dur' in matches[0]: # if the entry is a pulse entry
            matches[0]['pe_dur'] = float(matches[0]['pe_dur']) * unit_multiples[matches[0]['pe_dur_unit']]
            matches[0]['pe_delay_dur'] = float(matches[0]['pe_delay_dur']) * unit_multiples[matches[0]['pe_delay_unit']]

        matches[0]['sched_delay_unit'] = matches[0]['pe_dur_unit'] = matches[0]['pe_delay_unit'] = 's' 
    return sch_dict

def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", required=True, type=str, help="path to file containing ALARA output")
    arg = parser.parse_args()  
    return arg.f              

def main():
    lines = read_out(output_path)

    pulse_dict = read_pulse_histories(lines)
    sch_dict = make_nested_dict(lines)

    sch_dict = convert_to_s(sch_dict)

    out_path = parse_arg()

if __name__ == '__main__':
    main()
