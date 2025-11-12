import pandas as pd
import re
import argparse
from io import StringIO

class FileParser:

    def __init__(self, filepath: str):
        self.filepath = filepath
        self.results = {}

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
    def _is_new_block(line):
        return line.startswith('Interval #')

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
            Parse a block of table lines with StringIO into an ALARADFrame and
                store in the results dictionary.
            
            Arguments:
                self (alara_output_processing.FileParser): FileParser object.
                current_table_lines (list of str): Lines of the current table,
                    each stored as a separate string.
                results (dict): Dictionary that stores all parsed tables,
                    keyed by parameter and block name.
                current_parameter (str): Specific quantitative value
                    represented in the table (e.g. specific activity, number
                    density, etc.)
                current_interval (str): Interval iterated upon in ALARA run.

            Returns:
                None
            '''

            df = pd.read_csv(
                StringIO('\n'.join(current_table_lines)), sep=r'\s+'
            )

            df.columns = [c.replace('_', '') for c in df.columns]
            key = f'{current_parameter} - {current_block}'
            self.results[key] = ALARADFrame(df)

    def extract_tables(self):
        '''
        Reads an ALARA output file, identifies all data tables contained
            within, and stores each as an ALARADFrame in a dictionary.

        Arguments:
            self (alara_output_processing.FileParser): FileParser object.

        Returns:
            results (dict): Dictionary that stores all parsed tables,
                keyed by parameter and block name.
        '''

        with open(self.filepath, 'r') as f:
            lines = f.readlines()

        current_parameter = None
        current_block = None
        inside_table = False
        current_table_lines = []

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
                        self._parse_table_data(
                            current_table_lines,
                            current_parameter,
                            current_block
                        )
                    inside_table = False
                    current_table_lines = []
                continue

        return self.results
    
    # ---------- Output ----------
    def write_csv_files(self):
        '''
        Write out all DataFrames extracted from parsed ALARA output tables to
            their own CSV files.

        Arguments:
            self (alara_output_processing.FileParser): FileParser object.
        
        Returns:
            None
        '''

        for key, df in self.results.items():
            filename = self._sanitize_filename(key) + '.csv'
            df.to_csv(filename, index=False)

class ALARADFrame(pd.DataFrame):
    '''
    A subclass of pandas.DataFrame specialized for ALARA output.
    '''

    @property
    def _constructor(self):
        return ALARADFrame

    def process_time_vals(self, seconds=True):
        '''
        Convert the cooling times of the ALARA analysis post-shutdown to
            floating point numbers, in either seconds or years.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing the extracted tabular data for a
                single variable and interval/zone of an ALARA run.
            seconds (bool, optional): Option to convert cooling times from
                years to seconds.
                (Defaults to True)

        Returns:
            times (list): List of the ALARA cooling times, written as 
                floating point numbers of seconds or years.
        '''

        times = []
        time_dict = {'shutdown' : 0.0}
        time_dict['y'] = 365*24*60*60 if seconds else 1

        for column in self.columns[1:]:
            if column == 'shutdown':
                times.append(time_dict['shutdown'])
            else:
                time = column.split('y')[0]
                times.append(float(time) * time_dict['y'])

        return times
    
    def extract_totals(self):
        '''
        Select the values from the "total" row of an ALARA output table
            DataFrame and write them out to a list.
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing the extracted tabular data for a
                single variable and interval/zone of an ALARA run.

        Returns:
            totals (list): List of floating point numbers of the total values
                for the given response, with length equal to the number of
                cooling times.
        '''

        return self[self['isotope'] == 'total'].iloc[0, 1:].tolist()
    
    def filter_elements(self, elements):
        '''
        Create a new DataFrame containing only the data for nuclides of a
            selected element or elements.
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing the extracted tabular data for a
                single variable and interval/zone of an ALARA run.
            elements (str or list): Option to plot only the isotopes of a
                single element or list of selected elements.

        Returns:
            element_df (alara_output_processing.ALARADFrame): New ALARADFrame
                containing only rows for the selected element(s).
        '''

        if not isinstance(elements, list):
            elements = [elements]

        regex = '|'.join(fr'{el}-' for el in elements)

        return self[self['isotope'].str.contains(regex, case=False, na=False)]
    
    def relative_contributions(self):
        '''
        Create a new ALARADFrame representing the relative proportion of the
        given variable for the original ALARADFrame that each nuclide
        contributes at each time column. These values are relative to the
        total value at each time step, which may differ over time, depending
        on the variable.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing the extracted tabular data for a
                single variable and interval/zone of an ALARA run.

        Returns:
            adf_rel (alara_output_processing.ALARADFrame): New ALARADFrame
                containing each nuclides relative contribution to the total at
                each time step.                        
        '''

        totals = self.extract_totals()        
        adf_rel = self[self['isotope'] != 'total'].copy()
        for i, col in enumerate(adf_rel.columns[1:]):
            adf_rel[col] /= totals[i]

        return adf_rel
    
    def aggregate_small_percentages(self, relative=False, threshold=0.05):
        '''
        Consolidate all rows in an ALARADFrame that do not have any cells with
            a contribution of more than a threshold value to the total for its
            respective column. Rows that do not have any cells that surpass
            its column's threshold are aggregated into a new "Other" row. If a
            row has at least one column value above the threshold, then the
            whole row is preserved.
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing the extracted tabular data for a
                single variable and interval/zone of an ALARA run.
            relative (bool, optional): Option for DataFrames already processed
                by relative_contributions().
                (Defaults to False)     
            threshold (float or int): Proportional threshold value for
                inclusion cutoff.
                (Defaults to 0.05)
        Returns:
            mask_df (alara_output_processing.ALARADFrame): Processed
                ALARADFrame (potentially) with new row, "Other", containing
                all aggregrated data below the thresholds.
        '''

        cols = self.columns[1:]
        rel_adjustment = (
            [1] * len(cols) if relative else self.extract_totals()
            )
        threshold_vals = {
            col: threshold * adj for col, adj in zip(cols, rel_adjustment)
        }

        small_mask = (self[cols] < pd.Series(threshold_vals)).all(axis=1)
        other_row = pd.Series(0.0, index=self.columns, dtype=object)
        other_row[self.columns[0]] = 'Other'
        other_row[cols] = self.loc[small_mask, cols].sum()

        mask_adf = self.loc[~small_mask].reset_index(drop=True)
        if other_row[cols].sum() > 0:
            mask_adf = pd.concat(
                [mask_adf, pd.DataFrame([other_row])], ignore_index=True
            )

        return mask_adf

class DataLibrary:
    
    @staticmethod
    def make_entry(run_lbl, variable, unit, data):
        '''
        Construct a dictionary for a single ALARADFrame and its
            associated metadata.
        Arguments:
            run_lbl (str): Distinguisher between runs, such as different data
                sources, geometries, pulsing schedules, fluxes, etc. 
            variable (str): Dependent variable evaluated in DataFrame (i.e.
                Number Density, Specific Activity, etc.).
            unit (str): Associated units for the above variable (i.e. 
                atoms/kg, Bq/kg, etc.).
            data (alara_output_processing.ALARADFrame): ALARADFrame described
                by the above metadata.
        Returns:
            entry (dict): Single data entry for dictionary containing
                potentially multiple dataframes and associated metadata.
        '''

        return {
            f'{run_lbl} {variable}': {
                'Run Label': run_lbl,
                'Variable': variable,
                'Unit': unit,
                'Data': data
            }
        }

    @classmethod
    def make_entries(cls, runs_dict):
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
            cls (alara_output_processing.DataLibrary)
            runs_dict (dict): ALARA output data. If parsing directly from
                ALARA output files, runs_dict must be formatted as a
                dictionary of the form:
                runs_dict = {
                    Run Label 1 : path/to/output/file/for/data/run1,
                    Run Label 2 : path/to/output/file/for/data/run2
                }
                
        Returns:
            dfs (list of dicts): List of dictionaries containing ALARA output
                DataFrames and their metadata, of the form:
                df_dict = {
                    'Run Label'   : (Distinguisher between runs),
                    'Variable'    : (Any ALARA output variable, dependent on
                                    ALARA run parameters),
                    'Unit'        : (Respective unit matching the above
                                    variable),
                    'Data'        : (DataFrame containing ALARA output data
                                    for the given run parameter, variable, and
                                    unit)
                }
        '''
        
        dfs = {}
        for run_lbl, output_path in runs_dict.items():
            parser = FileParser(output_path)
            output_tables = parser.extract_tables()
            for key, data in output_tables.items():
                variable_w_unit, _ = key.split(' - ')
                variable, unit = variable_w_unit.split(' [')
                dfs.update(
                    cls.make_entry(run_lbl, variable, unit.strip(']'), data)
                )
        
        return dfs

###########################################

def args():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--filepath', '-f', required=True, nargs=1
    )
    return argparser.parse_args()

def main():
    parser = FileParser(args().filepath[0])
    parser.extract_tables()
    parser.write_csv_files()

if __name__ == '__main__':
    main()