import pandas as pd
import re
import argparse
import numpy as np
from io import StringIO
from warnings import warn
from csv import DictReader

# ---------- General Utility Methods ----------

SECONDS_CONV = {'s': 1}
SECONDS_CONV['m'] = 60  * SECONDS_CONV['s']
SECONDS_CONV['h'] = 60  * SECONDS_CONV['m']
SECONDS_CONV['d'] = 24  * SECONDS_CONV['h']
SECONDS_CONV['w'] = 7   * SECONDS_CONV['d']
SECONDS_CONV['y'] = 365 * SECONDS_CONV['d']
SECONDS_CONV['c'] = 100 * SECONDS_CONV['y']

def convert_times(vector, from_unit, to_unit):
    '''
    Convert the values in a vector to and from any units in the list of
        seconds (s), minutes (m), hours (h), days (d), weeks (w), years (y),
        or centuries (c).

    Arguments:
        vector (numpy.ndarray): 1-D NumPy array containing time values.
        from_unit (str): Time units of vector's input state. Accepted values:
            's', 'm', 'h', 'd', 'w', 'y', 'c'.
        to_unit (str): Time units of converted vector. Accepted values: 's',
            'm', 'h', 'd', 'w', 'y', 'c'.

    Returns:
        converted_vector (numpy.ndarray): 1-D Numpy array of time values
            converted to the desired units.
    '''

    return vector * (SECONDS_CONV[from_unit] / SECONDS_CONV[to_unit])

def extract_time_vals(times, to_unit='s'):
    '''
    Return the cooling times of an ALARADFrame from their column headers
        as a list in time units of the user's specification.

    Arguments:
        times (list): List of cooling times from an ALARA output table header.
        to_unit (str, optional): Time units for conversion. Accepted values:
            's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')

    Returns:
        times (numpy.ndarray): 1-D NumPy array of the converted ALARA cooling
            times.
    '''
    
    numeric_times = []
    for time in times:
        if 'shutdown' in time:
            numeric_times.append(0.0)
        else:
            time_str, unit = time.split('_')
            numeric_times.append(float(time_str))
    
    return convert_times(
        np.array(numeric_times), from_unit=unit, to_unit=to_unit
    )

def aggregate_small_percentages(adf_rel, threshold=0.05):
    '''
    Consolidate all nuclides that do not have any row-level values greater
        than a proportional threshold for totals over all times in a 
        pre-filtered ALARADFrame that has been operated on by
        ALARADFrame().calculate_relative_vals() already.
        Nuclides that do not have any values that surpass the absolute
        threshold at any time are aggregated into a new "Other" row for each
        time. If a nuclide has a value that surpasses the threshold at any
        time, then all rows for said nuclie are preserved.

    Arguments:
        adf_rel (alara_output_processing.ALARADFrame): ALARADFRame containing
            data from a single variable, response, and block that has been
            filtered and processed with ALARADFrame.calculate_relative_vals()
            prior. If those conditions are not met, a KeyError will be raised.
        threshold (float or int): Proportional threshold value for
            inclusion cutoff.
            (Defaults to 0.05)

    Returns:
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
    '''

    if 'rel_value' not in adf_rel.columns:
        raise KeyError(
            'Aggregation by small percentages requires calling '\
            'adf.calculate_relative_values() first.'
        )
    
    adf = adf_rel.copy()
    
    max_rel_by_nuc = adf.groupby('nuclide')['rel_value'].max()
    small_nuc = max_rel_by_nuc[max_rel_by_nuc < threshold].index
    large_nuc = max_rel_by_nuc[max_rel_by_nuc >= threshold].index

    large_adf = adf[adf['nuclide'].isin(large_nuc)]
    small_adf = adf[adf['nuclide'].isin(small_nuc)]

    if small_adf.empty:
        agg = large_adf
    else:
        other_rows = (small_adf.groupby('time').agg({
            'rel_value' : 'sum',
            'value' : 'sum',
            'run_lbl' : 'first',
            'block' : 'first',
            'block_num' : 'first',
            'variable' : 'first',
            'unit' : 'first'
            }).reset_index()
        )
        other_rows['nuclide'] = 'Other'
        agg = pd.concat([large_adf, other_rows], ignore_index=True)

    return agg

###########################################

class FileParser:

    def __init__(self, filepath, run_lbl):
        self.filepath = filepath
        self.run_lbl = run_lbl
        self.results = ALARADFrame()

    # ---------- Utility and Helper Methods ----------

    @staticmethod
    def _normalize_header(header_line: str):
        return re.sub(r'(\d+)\s+([a-zA-Z]+)', r'\1_\2', header_line)

    @staticmethod
    def _sanitize_filename(name: str):
        return re.sub(r'[<>:"/\\|?*\[\]\(\)\s]+', '_', name)

    @staticmethod
    def _is_new_parameter(line):
        return line.startswith('***') and line.endswith('***')

    @staticmethod
    def _is_new_interval(line):
        return line.startswith('Interval #')
    
    @staticmethod
    def _is_new_zone(line):
        return line.startswith('Zone #')
    
    @staticmethod
    def _is_new_material(line):
        return line.startswith('Material #')

    def _is_new_block(self, line):
        return (
            self._is_new_interval(line)
            or self._is_new_zone(line)
            or self._is_new_material(line)
        )

    @staticmethod
    def _is_table_header(line):
        return line.startswith('isotope')

    @staticmethod
    def _is_separator(line):
        return line.startswith('=')

    @staticmethod
    def _is_end_of_table(line):
        return line.startswith('total')

    # ---------- Core Parsing Logic ----------
    def _parse_table_data(
            self,
            current_table_lines,
            current_parameter,
            current_block
    ):
        '''
        Parse a block of table lines with StringIO into a csv.DictReader and
            store each row as a dictionary in a list of rows.
        
        Arguments:
            self (alara_output_processing.FileParser): FileParser object.
            current_table_lines (list of str): Lines of the current table,
                each stored as a separate string.
            results (dict): Dictionary that stores all parsed tables,
                keyed by parameter and block name.
            current_parameter (str): Specific quantitative value
                represented in the table (e.g. specific activity, number
                density, etc.)
            current_block (str): Block iterated upon in ALARA run.
        
        Returns:
            current_table_rows (list of dicts): List of each row in the
                table's rows as a dictionary of the form:
                row = {
                    'time' : Cooling time of data entry in seconds,
                    'nuclide' : Individual nuclide,
                    'run_lbl' : Distinguisher between runs,
                    'block' : ALARADFrame block integer enumerator,
                    'block_num' : Geometric position of block,
                    'variable' : ALARADFrame variable integer enumerator,
                    'unit' : Unit for corresponding variable,
                    'value' : Float value for the corresponding variable
                } 
        '''

        # Prepare table text for csv.DictReader
        text = '\n'.join(current_table_lines)
        comma_delim = re.sub(r'\t+|\s{2,}', ',', text.strip())
        
        reader = DictReader(StringIO(comma_delim), delimiter=',')
        
        # Handle and reformat DictReader data for row writing
        raw_cols = reader.fieldnames[:]
        nuclide_col = raw_cols[0]
        time_cols = raw_cols[1:]

        converted_times = extract_time_vals(time_cols)
        time_key_map = dict(zip(time_cols, converted_times))

        block_name, block_num_trail = current_block.split(' #')
        variable, unit = current_parameter.split(' [')

        current_table_rows = []
        for row in reader:
            for old_time, new_time in time_key_map.items():
                current_table_rows.append({
                    'time'           :                               new_time,
                    'nuclide'        :                       row[nuclide_col],
                    'run_lbl'        :                           self.run_lbl,
                    'block'          :     ALARADFrame.BLOCK_ENUM[block_name],
                    'block_num'      :          block_num_trail.split(' ')[0],
                    'variable'       :    ALARADFrame.VARIABLE_ENUM[variable],
                    'unit'           :                     unit.split(']')[0],
                    'value'          :                   float(row[old_time])
                })

        return current_table_rows

    def extract_tables(self):
        '''
        Reads an ALARA output file, identifies all data tables contained
            within, and stores all data in a singular ALARADFrame indexed by
            cooling time.

        Arguments:
            self (alara_output_processing.FileParser): FileParser object.
        
        Returns:
            self.results (alara_output_processing.ALARADFrame): Specialized
                ALARA output DataFrame containing all tabular data from the
                ALARA output file(s).
        '''

        with open(self.filepath, 'r') as f:
            lines = f.readlines()

        current_parameter = None
        current_block = None
        inside_table = False
        current_table_lines = []
        rows = []

        for line in lines:
            line = line.strip()

            if self._is_new_parameter(line):
                current_parameter = line.strip('* ').strip()
                continue

            if self._is_new_block(line):
                current_block = line.rstrip(':')
                continue

            if self._is_table_header(line):
                inside_table = True
                current_table_lines = [self._normalize_header(line)]
                continue

            if inside_table and self._is_separator(line):
                continue

            if inside_table:
                current_table_lines.append(line)
                if self._is_end_of_table(line):
                    if current_parameter and current_block:
                        rows.extend(self._parse_table_data(
                            current_table_lines,
                            current_parameter,
                            current_block
                        ))
                    inside_table = False
                    current_table_lines = []
                continue

        self.results = ALARADFrame(rows)

        if self.results.empty:
            warn(f'Unable to read tables from {self.filepath}')

        return self.results


class ALARADFrame(pd.DataFrame):
    '''
    A subclass of pandas.DataFrame specialized for ALARA output.
    '''

    VARIABLES = [
        'Number Density', 'Specific Activity', 'Total Decay Heat',
        'Alpha Heat', 'Beta Heat', 'Gamma Heat', 'Contact Dose'
        ]
    BLOCKS = ['Interval', 'Zone', 'Material']
    VARIABLE_ENUM = {name: i for i, name in enumerate(VARIABLES)}
    BLOCK_ENUM = {name: i for i, name in enumerate(BLOCKS)}

    @property
    def _constructor(self):
        return ALARADFrame

    def _filter_elements(self, elements):
        '''
        Create a new ALARADFrame containing only the data for nuclides of a
            selected element or elements to be called within filter_rows().
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            elements (str or list): Option to plot only the isotopes of a
                single element or list of selected elements.

        Returns:
            element_adf (alara_output_processing.ALARADFrame): New ALARADFrame
                containing only data for the selected element(s).
        '''

        regex = '|'.join(fr'{el}-' for el in elements)

        return self[self['nuclide'].str.contains(regex, case=False, na=False)]

    def filter_rows(self, filter_dict):
        '''
        Filter all rows that match user specifications for one or more
            columns.
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame object.
            filter_dict (dict): Dictionary of column-wise filtering 
                parameters. Multiple filters can be applied for a single
                column by providing the dictionary value as a list. The format
                of filter_dict is:
                filter_dict = {
                    Column 1 : Value or [Value1, ..., Value N],
                    ...
                    Column N : Value or [Value1, ..., Value N]
                }

        Returns:
            filtered_adf (alara_output_processing.ALARADFrame): Modified copy
                of self containing only rows that match all conditions in 
                filter_dict.
        '''
        
        filtered_adf = self.copy()
        for col_name, filters in filter_dict.items():
            if col_name in filtered_adf.columns:
                if not isinstance(filters, list):
                    filters = [filters]

                if col_name == 'nuclide':
                    elements, specific_nuclides = [], []
                    for f in filters:
                        if '-' not in f and f != 'total':
                            elements.append(f)
                        else:
                            specific_nuclides.append(f)

                    mask = pd.Series(False, index=filtered_adf.index)

                    if elements:
                        mask |= filtered_adf.index.isin(
                            filtered_adf._filter_elements(elements).index
                        )

                    if specific_nuclides:
                        mask |= filtered_adf[col_name].isin(specific_nuclides)

                else:
                    mask = filtered_adf[col_name].isin(filters)

                filtered_adf = filtered_adf[mask]
            
            else:
                warn(
                    f'Column name "{col_name}" from filter_dict not found in ' \
                    f'any of the ALARADFrame columns. Skipping filter for ' \
                    f'{col_name}.'
                )

        return filtered_adf.reset_index(drop=True)

    def calculate_relative_vals(self):
        '''
        Create a new column for a pre-filtered ALARADFrame with data for a
            single response, block, and run for relative contributions of each
            nuclide at each time to the total value of that response at each
            cooling time. If multiple runs, blocks, or variables are present
            in the ALARADFrame, a ValueError will be raised with directions
            for which column to further filter.

        Arguments:
            self (alara_output_processing.ALARADFrame): Pre-filtered
                ALARADFrame containing data for a single response, block, and
                run.

        Returns:
            adf_rel (alara_output_processing.ALARADFrame): Copy of self, with
                a new column 'rel_value' representing the relative
                contribution of each row's value to the corresponding total at
                each cooling time.
        '''

        necessary_single_filters = ['run_lbl', 'block', 'variable']
        for col in necessary_single_filters:
            if len(self[col].unique()) > 1:
                raise ValueError(
                    'Relative values can only be calculated for single run, '\
                    f'block, and variable ALARADFrames. \nFilter {col} to '\
                    'single value before calculating relative values.'
                )

        totals = self.filter_rows({'nuclide' : 'total'})['value'].to_numpy()
        times = sorted(set(self['time']))
        total_map = dict(zip(times, totals))
        adf_rel = self[self['nuclide'] != 'total'].copy()

        adf_rel['rel_value'] = adf_rel.apply(
            lambda row: row['value'] / total_map[row['time']],
            axis=1
        )

        return adf_rel


class DataLibrary:

    def __init__(self):
         self.adf = None

    def make_entries(self, runs_dict):
        '''
        Flexibly create a dictionary of subdictionaries containing
            ALARADFrames with associated metadata containing ALARA output data
            for different variables. Allows for processing of either existing
            ALARADFrames, with the requirement that the user provide the
            relevant run parameter, evaulated variable, and its relevant units
            to be packaged into a data dictionary. Alternatively, this
            function can directly read in an ALARA output file and parse all
            tables and their metadata internally.

        Arguments:
            self (alara_output_processing.DataLibrary): DataLibrary object.
            runs_dict (dict): ALARA output data. If parsing directly from
                ALARA output files, runs_dict must be formatted as a
                dictionary of the form:
                runs_dict = {
                    Run Label 1 : path/to/output/file/for/data/run1,
                    Run Label 2 : path/to/output/file/for/data/run2
                }
                
        Returns:
            self.adf (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing combined data from all tables in 
                all runs contained within runs_dict.
        '''

        dfs = []
        for run_lbl, output_path in runs_dict.items():
            parser = FileParser(output_path, run_lbl=run_lbl)
            dfs.append(parser.extract_tables())

        self.adf = ALARADFrame(pd.concat(dfs).fillna(0.0))

        return self.adf

###########################################

def args():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--filepath', '-f', required=True, nargs=1
    )
    argparser.add_argument(
        '--run_label', '-r', required=False, nargs=1
    )
    return argparser.parse_args()

def main():
    alara_data = FileParser(args().filepath[0], args().run_label[0])
    adf = alara_data.extract_tables()
    adf.to_csv(args().run_label[0])

if __name__ == '__main__':
    main()