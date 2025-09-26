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

def table_data(current_table_lines, results,
               current_parameter, current_block
    ):
        df = pd.read_csv(
            StringIO('\n'.join(current_table_lines)),
            delim_whitespace=True
        )

        df.columns = [c.replace("_", " ") for c in df.columns]
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
        stripped = line.strip()

        if stripped.startswith('***') and stripped.endswith('***'):
            current_parameter = stripped.strip('* ').strip()
            continue

        if stripped.startswith(
            'Interval #'
            ) or stripped.startswith('Totals for all intervals'):
            current_block = stripped.rstrip(':')
            continue

        if stripped.startswith('isotope'):
            inside_table = True
            current_table_lines = [normalize_header(stripped)]
            continue

        if inside_table and stripped.startswith('='):
            continue

        if inside_table and (
            not stripped or stripped.startswith(
                '***'
            ) or stripped.startswith('Interval')
        ):
            if len(
                current_table_lines
            ) > 1 and current_parameter and current_block:
                table_data(current_table_lines, results,
                           current_parameter, current_block
                )
            
            inside_table = False
            current_table_lines = []
            continue

        if inside_table:
            current_table_lines.append(stripped)

    if inside_table and len(
        current_table_lines
    ) > 1 and current_parameter and current_block:
        table_data(current_table_lines, results,
                   current_parameter, current_block
        )

    return results

def main():

    alara_tables = parse_tables(args().filepath[0])

    for key, df in alara_tables.items():
        filename = sanitize_filename(key) + '.csv'
        df.to_csv(filename, index=False)

if __name__ == '__main__':
    main()