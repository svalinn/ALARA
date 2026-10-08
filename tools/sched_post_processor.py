import argparse

unit_multipliers = {
    "c": 60 * 60 * 24 * 365 * 100,
    "y": 60 * 60 * 24 * 365,
    "w": 60 * 60 * 24 * 7,
    "d": 60 * 60 * 24,
    "h": 60 * 60,
    "m": 60,
    "s": 1,
}

def read_out(output_path):
    with open(output_path, "r") as output_file:
        lines = output_file.readlines()
    line_idx = 0
    while not lines[line_idx].startswith("top_schedule"):
        line_idx += 1
    return lines[line_idx:]


def read_pulse_histories(lines):
    """
    Creates a dictionary with the name of the pulse history as the key. The value is an iterable of tuples, where 
    each tuple has the form (# pulses (int), delay time (float), delay time unit ('s')). All pulse histories are
    stored regardless of usage in any schedule item.
    """
    pulse_dict = {}
    line_idx = 0
    while line_idx < len(lines):
        line = lines[line_idx].strip()
        if line.startswith("pulse_history"):
            num_pulse_line = lines[line_idx + 2].strip()
            delay_line = lines[line_idx + 3].strip()

            pulse_hist_name = line.split()[1].strip("':")
            nums_pulses = eval(num_pulse_line.split(":")[1])
            delays = eval(delay_line.split(":")[1])

            pulse_hist_list = []
            for num_pulse, delay in zip(nums_pulses, delays):
                pulse_hist_list.append(tuple([num_pulse, delay, 's']))
            pulse_dict[pulse_hist_name] = pulse_hist_list
            line_idx += 4
        else:
            line_idx += 1
    return pulse_dict


def make_sch_sub_dict(sch_line):
    sch_sub_dict = {
        "type" : "schedule",
        "sched_name": sch_line[1],
        "sched_ph_name": sch_line[3],
        "delay_dur": float(sch_line[5]) * unit_multipliers[sch_line[6]],
        "delay_unit": "s",
        "children" : [],
    }
    return sch_sub_dict


def make_pe_sub_dict(pe_line):
    pe_sub_dict = {
        "type" : "pulse_entry",
        "pulse_length": float(pe_line[1]) * unit_multipliers[pe_line[2]],
        "pulse_length_unit": "s",
        "corr_ph_name": pe_line[4],
        "delay_dur": float(pe_line[6]) * unit_multipliers[pe_line[7]],
        "delay_unit": "s",
    }
    return pe_sub_dict


def make_nested_dict(lines):
    """
    Constructs a hierarchy of dictionaries with a separate level for each additional tab found at the beginning of each line
    in the section of the output with schedule details.
    A sub-dictionary is created for each additional indented level.
    """
    top_schedule = "top_schedule"
    schedule = "schedule"
    pulse_entry = "pulse_entry:"
    sched_tree = {top_schedule : {"children": []}}
    line_idx = 0
    # next section of output
    current_sched = sched_tree[top_schedule]
    ancestors = [current_sched]
    
    keywords = [top_schedule, schedule, pulse_entry]

    while any(lines[line_idx].strip().startswith(keyword) for keyword in keywords):
        new_child_level = lines[line_idx].count("\t")
        tokens = lines[line_idx].strip().split()
        while new_child_level < len(ancestors):
            current_sched = ancestors.pop()

        if tokens[0] == top_schedule:
            sched_tree['top_schedule_name'] = tokens[1].strip(":'")

        elif tokens[0] == schedule:

            current_sched["children"].append(make_sch_sub_dict(tokens))
            ancestors.append(current_sched)
            current_sched = current_sched["children"][-1]

        elif tokens[0] == pulse_entry:
            current_sched["children"].append(make_pe_sub_dict(tokens))

        line_idx += 1
    return sched_tree

def add_ph_to_sch_tree(sch_tree, pulse_dict):
    """
    Combines the pulse history's information into the schedule dictionary, instead of only referring to the
    pulse histories in the schedule tree by name.
    """
    orig_sch_tree = sch_tree.copy() # Creates a static version of the nested dictionary for iteration
    for corr_ph_name in pulse_dict.keys():
        for value in orig_sch_tree.values():
            if value == corr_ph_name:
                sch_tree['pulse_history'] = pulse_dict[value]
            elif isinstance(value, dict):
                add_ph_to_sch_tree(value, pulse_dict)
            elif isinstance(value, list):
                for item in value:
                    if isinstance(item, dict):
                        add_ph_to_sch_tree(item, pulse_dict)
    return sch_tree


def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f",
                        "--filepath",
                        required=True,
                        type=str,
                        help="path to file containing ALARA output")
    parser.add_argument("-c",
                        "--combine_dicts",
                        default=False,
                        type=bool,
                        help="Add pulse history information into schedule dictionary")
    args = parser.parse_args()
    return args


def main():
    outputs = parse_arg()
    output_path = outputs.filepath
    to_combine = outputs.combine_dicts
    lines = read_out(output_path)

    pulse_dict = read_pulse_histories(lines)
    sch_tree = make_nested_dict(lines)

    if to_combine:
        sch_tree = add_ph_to_sch_tree(sch_tree, pulse_dict)


if __name__ == "__main__":
    main()
