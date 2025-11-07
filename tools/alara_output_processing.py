import pandas as pd
import re
import argparse
from io import StringIO

class TableParser:

    def __init__(self, filepath: str):
        self.filepath = filepath
        self.results = {}

    # ---------- Utility and Helper Methods ----------

    @staticmethod
    def normalize_header(header_line: str):
        return re.sub(r'(\d+)\s+([a-zA-Z]+)', r'\1_\2', header_line)

    @staticmethod
    def sanitize_filename(name: str):
        return re.sub(r'[<>:"/\\|?*\[\]\(\)\s]+', '_', name)

    @staticmethod
    def is_new_parameter(line):
        return line.startswith('***') and line.endswith('***')

    @staticmethod
    def is_new_block(line):
        return line.startswith('Interval #')

    @staticmethod
    def is_table_header(line):
        return line.startswith('isotope')

    @staticmethod
    def is_separator(line):
        return line.startswith('=')

    @staticmethod
    def is_end_of_table(line):
        return line.startswith('total')

    # ---------- Core Parsing Logic ----------
    def _parse_table_data(
            self,
            current_table_lines,
            current_parameter,
            current_block
        ):
            '''
            Parse a block of table lines with StringIO into a Pandas DataFrame
                and store in the results dictionary.
            
            Arguments:
                self
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
                StringIO('\n'.join(current_table_lines)),
                sep=r'\s+'
            )

            df.columns = [c.replace('_', '') for c in df.columns]
            key = f'{current_parameter} - {current_block}'
            self.results[key] = df

    def parse_output(self):
        '''
        Reads an ALARA output file, identifies all data tables contained
            within, and stores each as a Pandas DataFrame in a dictionary.

        Arguments:
            self

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

            if self.is_new_parameter(line):
                current_parameter = line.strip('* ').strip()
                continue

            if self.is_new_block(line):
                current_block = line.rstrip(':')
                continue

            if self.is_table_header(line):
                inside_table = True
                current_table_lines = [self.normalize_header(line)]
                continue

            if inside_table and self.is_separator(line):
                continue

            if inside_table:
                current_table_lines.append(line)
                if self.is_end_of_table(line):
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
            self
        
        Returns:
            None
        '''

        for key, df in self.results.items():
            filename = self.sanitize_filename(key) + '.csv'
            df.to_csv(filename, index=False)

class DataProcessing:
    def __init__(self):
        self.data_entries = {}
    
    # ---------- Utility Methods ----------
    
    @staticmethod
    def make_entry(datalib, variable, unit, data):
        '''
        Construct a dictionary entry for a single Pandas DataFrame and its
            associated metadata for updating into a larger data dictionary.
        Arguments:
            datalib (str): Data source for the DataFrame (i.e. fendl2,
                ALARAJOY-fendl3, etc.).
            variable (str): Dependent variable evaluated in DataFrame (i.e.
                Number Density, Specific Activity, etc.).
            unit (str): Associated units for the above variable (i.e. 
                atoms/kg, Bq/kg, etc.).
            data (pandas.core.frame.DataFrame): Pandas DataFrame described by
                the above metadata.
        Returns:
            entry (dict): Single data entry for dictionary containing
                potentially multiple dataframes and associated metadata.
        '''

        return {
            f'{datalib} {variable}': {
                'Data Source': datalib,
                'Variable': variable,
                'Unit': unit,
                'Data': data
            }
        }
    
    @staticmethod
    def process_time_vals(df, seconds=True):
        '''
        Convert the cooling times of the ALARA analysis post-shutdown to
            floating point numbers, in either seconds or years.

        Arguments:
            df (pandas.core.frame.DataFrame): DataFrame containing the
                extracted tabular data for a single variable and interval/zone
                of an ALARA run.
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

        for column in df.columns[1:]:
            if column == 'shutdown':
                times.append(time_dict['shutdown'])
            else:
                time = column.split('y')[0]
                times.append(float(time) * time_dict['y'])

        return times

    @staticmethod
    def extract_totals(df):
        '''
        Select the values from the "total" row of an ALARA output table
            DataFrame and write them out to a list.
        
        Arguments:
            df (pandas.core.frame.DataFrame): DataFrame containing the
                extracted tabular data for a single variable and interval/zone
                of an ALARA run.

        Returns:
            totals (list): List of floating point numbers of the total values
                for the given response, with length equal to the number of
                cooling times.
        '''

        return df[df['isotope'] == 'total'].iloc[0, 1:].tolist()
    
    @staticmethod
    def filter_elements(df, elements):
        '''
        Create a new DataFrame containing only the data for nuclides of a
            selected element or elements.
        
        Arguments:
            df (pandas.core.frame.DataFrame): DataFrame containing the
                extracted tabular data for a single variable and interval/zone
                of an ALARA run.
            elements (str or list): Option to plot only the isotopes of a
                single element or list of selected elements.

        Returns:
            element_df (pandas.core.frame.DataFrame): New DataFrame containing
                only rows for the selected element(s).
        '''

        if not isinstance(elements, list):
            elements = [elements]

        regex = '|'.join(fr'{el}-' for el in elements)
        
        return df[df['isotope'].str.contains(regex, case=False, na=False)]
    
    def aggregate_small_percentages(self, df, relative=False, threshold=0.05):
        '''
        Consolidate all rows in a DataFrame that do not have any cells with a
            contribution of more than a threshold value to the total for its
            respective column. Rows that do not have any cells that surpass
            its column's threshold are aggregated into a new "Other" row. If a
            row has at least one column value above the threshold, then the
            whole row is preserved.
        Arguments:
            df (pandas.core.frame.DataFrame): DataFrame containing the
                extracted tabular data for a single variable and interval/zone
                of an ALARA run.
            relative (bool, optional): Option for DataFrames already processed
                by relative_contributions().
                (Defaults to False)     
            threshold (float or int): Proportional threshold value for
                inclusion cutoff.
                (Defaults to 0.05)
        Returns:
            mask_df (pandas.core.frame.DataFrame): Processed DataFrame
                (potentially) with new row, "Other", containing all
                aggregrated data below the thresholds.
        '''

        cols = df.columns[1:]
        rel_adjustment = (
            [1] * len(cols) if relative else self.extract_totals(df)
            )
        threshold_vals = {
            col: threshold * adj for col, adj in zip(cols, rel_adjustment)
        }

        small_mask = (df[cols] < pd.Series(threshold_vals)).all(axis=1)
        other_row = pd.Series(0.0, index=df.columns, dtype=object)
        other_row[df.columns[0]] = 'Other'
        other_row[cols] = df.loc[small_mask, cols].sum()

        mask_df = df.loc[~small_mask].reset_index(drop=True)
        if other_row[cols].sum() > 0:
            mask_df = pd.concat(
                [mask_df, pd.DataFrame([other_row])], ignore_index=True
            )

        return mask_df
    
    # ---------- Core Processing ----------

    def process_data(
            self,
            data_source,
            inp_datalib=None,
            inp_variable=None,
            inp_unit = None
    ):
        '''
        Flexibly create a dictionary of subdictionaries containing Pandas
            DataFrames with associated metadata containing ALARA output data
            for different variables. Allows for processing of either existing
            DataFrames, with the requirement that the user provide the
            relevant data source, evaulated variable, and its relevant units
            to be packaged into a data dictionary. Alternatively, this
            function can directly read in an ALARA output file and parse all
            tables and their metadata internally.

        Arguments:
            data_source (dict or pandas.core.frame.DataFrame): ALARA output
                data. If parsing directly from ALARA output files, data_source
                must be formatted as a dictionary of the form:
                data_source = {
                    Data Library 1 : path/to/output/file/for/data/library1,
                    Data Library 2 : path/to/output/file/for/data/library2
                }
                If processing a preexisting DataFrame, then data_source is
                just the DataFrame itself.
            inp_datalib (str or None, optional): Data source of the selected
                DataFrame. Required if processing a preexisting DataFrame;
                irrelevant if parsing directly from ALARA output files.
                (Defaults to None)
            inp_variable (str or None, optional): Evaluated variable for the
                selected DataFrame. Required if processing a preexisting
                DataFrame; irrelevant if parsing directly from ALARA output
                files.
                (Defaults to None)
            inp_unit (str or None, optional): Appropriate unit for the
                evaluated variable for the selected DataFrame. Required if
                processing a preexisting DataFrame; irrelevant if parsing
                directly from ALARA output files.
                (Defaults to None)
                
        Returns:
            dfs (list of dicts): List of dictionaries containing ALARA output
                DataFrames and their metadata, of the form:
                df_dict = {
                    'Data Source' : (Either 'fendl2' or 'fendl3'),
                    'Variable'    : (Any ALARA output variable, dependent on
                                    ALARA run parameters),
                    'Unit'        : (Respective unit matching the above
                                    variable),
                    'Data'        : (DataFrame containing ALARA output data
                                    for the given data source, variable, and
                                    unit)
                }
        '''
        
        dfs = {}

        if isinstance(data_source, pd.DataFrame):
            dfs.update(
                self.make_entry(
                    inp_datalib, inp_variable, inp_unit, data_source
                )
            )
            return dfs
        
        for datalib, output_path in data_source.items():
            parser = TableParser(output_path)
            output_tables = parser.parse_output()
            for key, data in output_tables.items():
                variable_w_unit, _ = key.split(' - ')
                variable, unit = variable_w_unit.split(' [')
                dfs.update(
                    self.make_entry(datalib, variable, unit.strip(']'), data)
                )
        
        self.data_entries.update(dfs)
        return dfs

###########################################

def args():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--filepath', '-f', required=True, nargs=1
    )
    return argparser.parse_args()

def main():
    parser = TableParser(args().filepath[0])
    parser.parse_output()
    parser.write_csv_files()

if __name__ == '__main__':
    main()