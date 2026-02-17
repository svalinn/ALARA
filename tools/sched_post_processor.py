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
    Creates a dictionary with the name of the pulse history as the key, and a dictionary containing the number of pulses
    in all pulsing levels & the delay (in seconds) of all pulsing levels as the value. All pulse histories are stored
    regardless of usage in any schedule item.
    """
    pulse_dict = {}
    line_idx = 0
    while line_idx < len(lines):
        line = lines[line_idx].strip()
        if line.startswith("pulse_history"):
            num_pulse_line = lines[line_idx + 2].strip()
            delay_line = lines[line_idx + 3].strip()

            pulse_hist_name = line.split()[1].strip("':")
            num_pulses = num_pulse_line.split(":")[1]
            delays = delay_line.split(":")[1]

            pulse_dict[pulse_hist_name] = {
                "num_pulses_all_levels": num_pulses,
                "delay_seconds_all_levels": delays,
            }
            line_idx += 4
        else:
            line_idx += 1
    return pulse_dict


def make_sch_sub_dict(sch_line):
    sch_sub_dict = {
        "type" : "schedule",
        "sched_name": sch_line[1],
        "sched_ph_name": sch_line[3],
        "sched_delay_dur": float(sch_line[5]) * unit_multipliers[sch_line[6]],
        "sched_delay_unit": "s",
        "children" : [],
    }
    return sch_sub_dict


def make_pe_sub_dict(pe_line, unit_multipliers):
    pe_sub_dict = {
        "type" : "pulse_entry",
        "pe_dur": float(pe_line[1]) * unit_multipliers[pe_line[2]],
        "pe_dur_unit": "s",
        "corr_ph_name": pe_line[4],
        "pe_delay_dur": float(pe_line[6]) * unit_multipliers[pe_line[7]],
        "pe_delay_unit": "s",
    }
    return pe_sub_dict


def make_nested_dict(lines):
    """
    Constructs a hierarchy of dictionaries with a separate level for each additional tab found at the beginning of each line
    in the section of the output with schedule details.
    A sub-dictionary is created for each additional indented level.
    """
    sched_tree = {0: {"children" : [] } }
    line_idx = 0
    # next section of output
    ancestors = []
    current_sched = sched_tree[0]

    while not lines[line_idx].startswith("pulse_history:"):
        new_child_level = lines[line_idx].count("\t")
        tokens = lines[line_idx].strip().split()

        if "children" not in current_sched.keys():
            current_sched["children"] = []

        while new_child_level < len(ancestors):
            current_sched = ancestors.pop()

        if tokens[0] == "schedule":

            current_sched["children"].append(make_sch_sub_dict(tokens))
            ancestors.append(current_sched)
            current_sched = current_sched["children"][-1]

        elif tokens[0] == "top_schedule":
            ancestors.append(current_sched)

        elif tokens[0] == "pulse_entry:":
            current_sched["children"].append(make_pe_sub_dict(tokens))

        line_idx += 1

    return sch_dict


def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f",
                        required=True,
                        type=str,
                        help="path to file containing ALARA output")
    arg = parser.parse_args()
    return arg.f


def main():
    output_path = parse_arg()
    lines = read_out(output_path)

    pulse_dict = read_pulse_histories(lines)
    sch_dict = make_nested_dict(lines)


if __name__ == "__main__":
    main()
