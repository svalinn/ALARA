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
                    'time' : Cooling time of data entry in seconds,
                    'nuclide' : Individual nuclide,
                    'run_lbl' : Distinguisher between runs,
                    'block' : ALARADFrame block integer enumerator,
                    'block_num' : Geometric position of block,
                    'variable' : ALARADFrame variable integer enumerator,
                    'value' : Float value for the corresponding variable
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
        adf.columns = adf.extract_time_vals(parsing=True)

        for isotope in adf.index:
            for time in adf.columns:
                current_table_rows.append({
                    'time'                 :                             time,
                    'nuclide'              :                          isotope,
                    'run_lbl'              :                     self.run_lbl,
                    'block'                :       adf.BLOCK_ENUM[block_name],
                    'block_num'            :    block_num_trail.split(' ')[0],
                    'variable'             :      adf.VARAIBLE_ENUM[variable],
                    'value'                :           adf.loc[isotope, time]
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

    VARIABLES = [
        'Number Density', 'Specific Activity', 'Total Decay Heat',
        'Alpha Heat', 'Beta Heat', 'Gamma Heat', 'Contact Dose'
        ]
    BLOCKS = ['Interval', 'Zone', 'Material']
    VARAIBLE_ENUM = {name: i for i, name in enumerate(VARIABLES)}
    BLOCK_ENUM = {name: i for i, name in enumerate(BLOCKS)}

    @property
    def _constructor(self):
        return ALARADFrame
    
    def _operate_inplace(self, inplace=False):
        '''
        Internal method to establish whether subsequent ALARADFrame operations
            act on the ALARADFrame itself or produce a copy frame.
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            inplace (bool, optional): Option to modify the ALARADFrame or
                create a new one.

        Returns:
            adf (alara_output_processing.ALARADFrame): Either the original
                ALARADFrame or a copy.

        '''

        return self if inplace else self.copy()

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

    def filter_rows(self, filter_dict, inplace=False):
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
            inplace (bool, optional): Option to modify the ALARADFrame or
                create a new one.

        Returns:
            filtered_adf (alara_output_processing.ALARADFrame): Modified copy
                of self containing only rows that match all conditions in 
                filter_dict.
        '''
        
        filtered_adf = self._operate_inplace(inplace)
        for col_name, filters in filter_dict.items():
            if not isinstance(filters, list):
                filters = [filters]

            # Special wrapping of filter_elements() to select all nuclides of
            # a given element or elements without having to explicitly list
            # each nuclide present from that element or list of elements
            if (
                col_name == 'nuclide'
                and any('-' not in f and f != 'total' for f in filters)
            ):
                    filtered_adf = filtered_adf._filter_elements(filters)
                    continue

            mask = filtered_adf[col_name].isin(filters)
            filtered_adf = filtered_adf[mask]
        
        return filtered_adf.reset_index(drop=True)

    def extract_time_vals(self, parsing=False):
        '''
        Return the cooling times of an ALARADFrame from their column headers
            as a list in seconds.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            parsing (bool, optional): Option for ALARADFrame format when
                calling this funcion inside FileParser._parse_table_data().
                (Defaults to False)

        Returns:
            times (list): List of the ALARA cooling times, written as 
                floating point numbers of seconds.
        '''

        if not parsing:
            return sorted(set(self['time']))
    
        else:
            times = []
            time_dict = { 'shutdown' : 0.0, 'y' : 365*24*60*60 }

            for time in self.columns:
                if time == 'shutdown':
                    times.append(time_dict['shutdown'])
                else:
                    times.append(float(time.split('y')[0]) * time_dict['y'])
            
            return times
        
    def convert_times(self, unit, inplace=False, pivot=False):
        '''
        Convert the values in the ALARADFrame rows or columns from seconds to
            minutes (m), hours (h), days (d), weeks (w), years (y), or
            centuries (c).

        Arguments:
            self (alara_output_processing.ALARADFrame): ALARADFrame on which
                to operate. If operating on a pivot table, make sure to
                convert to an ALARADFrame first from pd.DataFrame.
            unit (str): Time unit for time value conversion. Accepted values:
                m, h, d, w, y, c.
            inplace (bool, optional): Option to modify the ALARADFrame or
                create a new one.
            pivot (bool, optional): Option for operation on a pivot table.
            (Defaults to False) 
        '''

        conv_dict = {'m' : 60}
        conv_dict['h'] = 60  * conv_dict['m']
        conv_dict['d'] = 24  * conv_dict['h']
        conv_dict['w'] = 7   * conv_dict['d']
        conv_dict['y'] = 365 * conv_dict['d']
        conv_dict['c'] = 100 * conv_dict['y']

        adf = self._operate_inplace(inplace)
        times = adf.columns if pivot else adf['time']
        times /= conv_dict[unit]

        return adf

    def extract_totals(self):
        '''
        Extract a list of the totals for a single response variable at each
            cooling time from an ALARADFrame with data from a single run and
            response variable.
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.

        Returns:
            totals (list of floats or None): List of total values at each
                cooling time in the ALARADFrame. Returns empty list if 'total'
                row not present in the ALARADFrame.
        '''

        try:
            totals = self.filter_rows({'nuclide : total'})['value'].tolist()
        
        except AttributeError:
            totals = self[self.index == 'total'].iloc[0].tolist()

        else:
           warn('ALARADFrame missing "total" row.')
           totals = []
        
        return totals


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

def aggregate_small_percentages(piv, relative=False, threshold=0.05):
    '''
    Consolidate all rows in a pre-filtered and pivoted DataFrame derived from
        a larger ALARADFRame that do not have any cells with a contribution of
        more than a threshold value to the total for its respective column.
        Rows that do not have any cells that surpass its column's threshold
        are aggregated into a new "Other" row. If a row has at least one
        column value above the threshold, then the whole row is preserved.

    Arguments:
        piv (pd.DataFrame): DataFrame created from the pivoting of an
            ALARADFrame already operated on by ALARADFrame.filter_rows().
        relative (bool, optional): Option for ALARADFrames already
            processed by relative_contributions().
            (Defaults to False)     
        threshold (float or int): Proportional threshold value for
            inclusion cutoff.
            (Defaults to 0.05)

    Returns:
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new row, "Other", containing all aggregated
            data below the thresholds.
    '''

    piv = ALARADFrame(piv)
    cols = piv.columns
    
    rel_adjustment = (
        [1] * len(cols) if relative else piv.extract_totals()
    )
    threshold_vals = {
        col: threshold * adj for col, adj in zip(cols, rel_adjustment)
    }
    piv = piv[piv.index != 'total']

    small_mask = (piv[cols] < pd.Series(threshold_vals)).all(axis=1)
    other_row = pd.Series(piv.loc[small_mask, cols].sum(), name='Other')
    agg = piv.loc[~small_mask.values]

    if other_row[cols].sum() > 0:
        agg = pd.concat([agg, other_row.to_frame().T])
    agg.index.name = 'nuclide'

    return agg

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