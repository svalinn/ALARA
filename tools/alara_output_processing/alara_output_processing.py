import pandas as pd
import re
import argparse
from io import StringIO
from warnings import warn

class FileParser:

    def __init__(self, filepath, run_lbl):
        self.filepath = filepath
        self.run_lbl = run_lbl
        self.rows = []
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
        Parse a block of table lines with StringIO into an ALARADFrame and
            store each row as a single element in a list of rows.
        
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
                    'Time' : Cooling time of data entry in seconds,
                    'Nuclide' : Individual nuclide,
                    'Run Parameter' : Distinguisher between runs,
                    'Block' : 'Interval', 'Zone', or 'Material',
                    'Block Type' : Corresponding integer enumerator,
                    'Block Number' : Geometric position of block,
                    'Variable' : Name of the ALARA response variable
                    'Variable Type' : Corresponding integer enumerator,
                    'Value' : Float value for the corresponding variable
                } 
        '''

        current_table_rows = []
        df = pd.read_csv(
            StringIO('\n'.join(current_table_lines)), sep=r'\s+'
        ).set_index('isotope')

        df.columns = [c.replace('_', '') for c in df.columns]
        block_name, block_num_trail = current_block.split(' #')
        variable = current_parameter.split(' [')[0]
        adf = ALARADFrame(df)
        adf.columns = adf.process_time_vals(parsing=True)

        for isotope in adf.index:
            for time in adf.columns:
                current_table_rows.append({
                    'Time'                 :                             time,
                    'Nuclide'              :                          isotope,
                    'Run Parameter'        :                     self.run_lbl,
                    'Block'                :                       block_name,
                    'Block Type'           :       adf.BLOCK_ENUM[block_name],
                    'Block Number'         :    block_num_trail.split(' ')[0],
                    'Variable'             :                         variable,
                    'Variable Type'        :      adf.VARAIBLE_ENUM[variable],
                    'Value'                :           adf.loc[isotope, time]
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
                        self.rows.extend(self._parse_table_data(
                            current_table_lines,
                            current_parameter,
                            current_block
                        ))
                    inside_table = False
                    current_table_lines = []
                continue
        
        self.results = ALARADFrame(self.rows)

        if self.results.empty:
            warn(f'Unable to read tables from {self.filepath}')

        return self.results


class ALARADFrame(pd.DataFrame):
    '''
    A subclass of pandas.DataFrame specialized for ALARA output.
    '''
    _metadata = ['is_pivot', 'is_single_var', 'is_single_run']
    VARIABLES = ['Number Density', 'Specific Activity', 'Total Decay Heat', 'Contact Dose']
    BLOCKS = ['Interval', 'Zone', 'Material']
    VARAIBLE_ENUM = {name: i for i, name in enumerate(VARIABLES)}
    BLOCK_ENUM = {name: i for i, name in enumerate(BLOCKS)}

    @property
    def _constructor(self):
        return ALARADFrame
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.is_pivot = False
        self.is_single_var = False
        self.is_single_run = False

    def select_single_var(self, variable):
        '''
        Pivot an ALARADFrame object to contain the data for a single response
            variable. Reindex the new ALARADFrame by nuclide, with cooling
            time and run parameter being a new pd.MultiIndex column structure.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing all of the combined output data
                of all runs and responses from the FileParser.

            variable (str): Single response variable to select for new
                ALARADFrame.
            
        Returns:
            single_var_adf (alara_output_processing.ALARADFrame): New pivoted
                ALARADFrame containing only a single response variable's data
                indexed by nuclide with a pd.MultiIndex column structure of 
                cooling time and run parameter.    
        '''

        single_var_adf = self[self['Variable'] == variable]
        single_var_adf = ALARADFrame(single_var_adf.pivot(
            index='Nuclide',
            columns=['Time', 'Run Parameter'],
            values='Value'
        )).fillna(0.0)

        single_var_adf.is_pivot = True
        single_var_adf.is_single_var = True

        return single_var_adf

    def select_single_run(self, run_lbl):
        '''
        Select only the columns of an ALARADFrame containing data from a
            single run. Can operate on either a full ALARADFrame indexed by
            time or a pivoted, single-variable ALARADFrame indexed by nuclide.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame object.
            run_lbl (str): Single run parameter to isolate in new ALARADFrame.

        Returns:
            single_run_adf (alara_output_processing.ALARADFrame): ALARADFrame
                containing only the data from a single run.
        '''

        try:
            single_run_adf = self[self['Run Parameter'] == run_lbl]

        except KeyError:
            single_run_adf = self.loc[:, pd.IndexSlice[:, run_lbl]]
            single_run_adf.is_pivot = True
        
        single_run_adf.is_single_run = True

        return single_run_adf

    def process_time_vals(self, parsing=False, seconds=True):
        '''
        Convert the cooling times of an ALARADFrame to floating point numbers,
            in either seconds or years.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            parsing (bool, optional): Option for ALARADFrame format when
                calling this funcion inside FileParser._parse_table_data().
                (Defaults to False)
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

        if parsing:
            cooling_times = self.columns
        elif self.is_pivot:
            cooling_times = self.columns.get_level_values('Time')
        else:
            cooling_times = set(self['Time'])

        for time in cooling_times:
            if time == 'shutdown':
                times.append(time_dict['shutdown'])
            else:
                times.append(float(time.split('y')[0]) * time_dict['y'])

        return sorted(set(times))

    def extract_totals(self, variable=None, run_lbl=None):
        '''
        Extract a list of the totals for a single response variable at each
            cooling time from an ALARADFrame with data from a single run and
            response variable. ALARADFrames containing multiple runs and/or
            variables can be internally specified through the input
            parameters, but failure to input optional parameters for multi-run
            or multivariable ALARADFrames will be unsuccessful in identifying
            which set of totals to return.
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            variable (str, optional): Option to specify the single variable
                to extract its totals. Unnecessary for ALARADFrames that
                already contain only one variable.
                (Defaults to None)
            run_lbl (str, optional): Option to specify the single run for the
                variable's totals at each cooling time. Unnecessary for
                ALARADFrames that already contain only one run.
                (Defaults to None)

        Returns:
            totals (list of floats or None): List of total values at each
                cooling time in the ALARADFrame. Returns None if insufficient
                parameters are defined for ALARADFrames containing multiple
                runs and/or variables.
        '''

        single_run = ALARADFrame()
        runs = (
            set(self.columns.get_level_values('Run Parameter'))
            if self.is_pivot else set(self['Run Parameter'])
            )
        if len(runs) == 1:
            run_lbl = list(runs)[0]
        
        if self.is_single_var or variable:
            single_var = (
                self.copy() if self.is_pivot
                else self.select_single_var(variable)
            )

            single_run = single_var.select_single_run(run_lbl)
            return single_run[single_run.index == 'total'].iloc[0].tolist()

        # Prompt to define missing specificiations, returns None    
        warn('Must specify run parameter and variable.')

    def filter_elements(self, elements):
        '''
        Create a new ALARADFrame containing only the data for nuclides of a
            selected element or elements.
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            elements (str or list): Option to plot only the isotopes of a
                single element or list of selected elements.

        Returns:
            element_adf (alara_output_processing.ALARADFrame): New ALARADFrame
                containing only data for the selected element(s).
        '''

        if not isinstance(elements, list):
            elements = [elements]

        regex = '|'.join(fr'{el}-' for el in elements)
        nuclides = self.index if self.is_pivot else self['Nuclide']

        return self[nuclides.str.contains(regex, case=False, na=False)]

    def aggregate_small_percentages(
            self, run_lbl=None, relative=False, threshold=0.05
    ):
        '''
        Consolidate all rows in an ALARADFrame that do not have any cells with
            a contribution of more than a threshold value to the total for its
            respective column. Rows that do not have any cells that surpass
            its column's threshold are aggregated into a new "Other" row. If a
            row has at least one column value above the threshold, then the
            whole row is preserved. ALARADFrames that contain multiple runs
            require specification of which run to aggregate.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            run_lbl (str, optional): Option to specify which run parameter to
                select. Necessary if multiple runs present in ALARADFrame.
                (Defaults to None)
            relative (bool, optional): Option for ALARADFrames already
                processed by relative_contributions().
                (Defaults to False)     
            threshold (float or int): Proportional threshold value for
                inclusion cutoff.
                (Defaults to 0.05)
        Returns:
            agg_adf (alara_output_processing.ALARADFrame): Processed
                ALARADFrame (potentially) with new row, "Other", containing
                all aggregrated data below the thresholds.
        '''

        single_run_adf = (
            self.copy() if self.is_single_run
            else self.select_single_run(run_lbl)
        ).fillna(0.0)
        
        if isinstance(single_run_adf.columns, pd.MultiIndex):
            single_run_adf.is_single_var = True
        else:
            warn('Aggregation only valid for single variable ALARADFrames')
            return None
        
        time_idx = single_run_adf.columns.get_level_values('Time')
        rel_adjustment = (
            [1] * len(time_idx) if relative
            else single_run_adf.extract_totals()
            )
        single_run_adf = single_run_adf[single_run_adf.index != 'total']
        threshold_vals = {
            col: threshold * adj for col, adj in zip(time_idx, rel_adjustment)
        }

        threshold_series = pd.Series(
            [threshold_vals[t] for t in time_idx],
            index=single_run_adf.columns
        )

        small_mask = (single_run_adf < threshold_series).all(axis=1)
        other_row = pd.Series(0.0, index=single_run_adf.columns, dtype=object)
        time_cols = single_run_adf.columns[time_idx.isin(time_idx)]
        other_row[time_cols] = single_run_adf.loc[small_mask, time_cols].sum()
        other_row.name=  'Other'
        agg_adf = single_run_adf.loc[~small_mask.values]

        if other_row[time_cols].sum() > 0:
            agg_adf = pd.concat([agg_adf, pd.DataFrame([other_row])])

        return agg_adf


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