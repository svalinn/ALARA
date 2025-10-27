import pandas as pd
import re
import argparse
from io import StringIO

def args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--filepath', '-f', required=True, nargs=1
    )
    return parser.parse_args()

def normalize_header(header_line=str) -> str:
    return re.sub(r'(\d+)\s+([a-zA-Z]+)', r'\1_\2', header_line)

def sanitize_filename(name: str) -> str:
    return re.sub(r'[<>:"/\\|?*\[\]\(\)\s]+', '_', name)

def is_new_parameter(line):
    return line.startswith('***') and line.endswith('***')

def is_new_block(line):
    return (
        line.startswith('Interval #')
        or line.startswith('Totals for all intervals')
        )

def is_table_header(line):
    return line.startswith('isotope')

def is_separator(line):
    return line.startswith('=')

def is_end_of_table(line):
    return line.startswith('total')

def table_data(
        current_table_lines,
        results,
        current_parameter,
        current_block
    ):
        df = pd.read_csv(
            StringIO('\n'.join(current_table_lines)),
            sep=r'\s+'
        )

        df.columns = [c.replace('_', '' '') for c in df.columns]
        key = f'{current_parameter} - {current_block}'
        results[key] = df

def parse_tables(filename):
    results = {}
    with open(filename, "r") as f:
        lines = f.readlines()

    current_parameter = None
    current_block = None
    inside_table = False
    current_table_lines = []

    for line in lines:
        line = line.strip()

        if is_new_parameter(line):
            current_parameter = line.strip('* ').strip()
            continue

        if is_new_block(line):
            current_block = line.rstrip(':')
            continue

        if is_table_header(line):
            inside_table = True
            current_table_lines = [normalize_header(line)]
            continue

        if is_separator(line):
            continue

        if inside_table:
            current_table_lines.append(line)
            if is_end_of_table(line):
                if current_parameter and current_block:
                    table_data(
                        current_table_lines,
                        results,
                        current_parameter,
                        current_block
                    )
                
                inside_table = False
                current_table_lines = []
            continue

        if inside_table:
            current_table_lines.append(line)

    if inside_table and current_parameter and current_block:
        table_data(
            current_table_lines,
            results,
            current_parameter,
            current_block
        )

    return results

def main():

    alara_tables = parse_tables(args().filepath[0])

    for key, df in alara_tables.items():
        filename = sanitize_filename(key) + '.csv'
        df.to_csv(filename, index=False)

if __name__ == '__main__':
    main()