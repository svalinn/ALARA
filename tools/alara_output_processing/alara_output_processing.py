import pandas as pd
import argparse
import pypact
from operator import gt, lt, ge
from warnings import warn
from csv import DictReader
from numpy import array
from pathlib import Path


# ---------- General Utility Methods ----------

OPS = { '>' : gt, '<' : lt , '>=' : ge }

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
        converted_list (list): Chronologically sorted list of the converted
            ALARA cooling times.
    '''

    return sorted(vector * (SECONDS_CONV[from_unit] / SECONDS_CONV[to_unit]))

def extract_time_vals(cols, to_unit='s'):
    '''
    Return the cooling times of an ALARADFrame from their column headers
        as a list in time units of the user's specification.

    Arguments:
        cols (list): List of cooling times and respective units from an ALARA
            output table header. Units can be different for each time, but
            will all be converted to the same to_unit. 
        to_unit (str, optional): Time units for conversion. Accepted values:
            's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')

    Returns:
        times (list): Chronologically sorted list of the converted ALARA
            cooling times in the selected units.
    '''
    
    converted_times = []

    if cols[0] == 'pre-irrad':
        converted_times.append(-1.0)
        cols = cols[1:]
    
    # If "shutdown" is present in the time columns, it will necessarily be the
    # 0th entry. "Shutdown" is unit-ambivalent at time=0.
    if cols[0] == 'shutdown':
        converted_times.append(0.0)
        cols = cols[1:]

    for time, unit in zip(cols[::2], cols[1::2]):
        converted_times.append(convert_times(
            array([float(time)]), from_unit=unit, to_unit=to_unit
        )[0])

    return converted_times

def aggregate_small_percentages(adf, threshold):
    '''
    Consolidate all nuclides that do not have any row-level values greater
        than a given threshold for totals over all times in a pre-filtered
        ALARADFrame containing data from a single variable, response, and
        block. The 'value' column in the ALARADFrame can either contain
        absolute or relative data, requiring only that the user provide an
        appropriate threshold value.

        Nuclides that do not contribute any values that surpass the threshold
        at any time are aggregated into a new "Other" row for each time. If a
        nuclide has a value that surpasses the threshold at any time, then all
        rows for said nuclide are preserved.

    Arguments:
        adf (alara_output_processing.ALARADFrame): ALARADFRame containing
            data from a single variable, response, and block. Can contain
            either absolute or relative response data.
        threshold (float or int): Threshold value for inclusion cutoff, either
            absolute or relative.

    Returns:
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
    '''

    adfc = adf.copy()
    
    max_rel_by_nuc = adfc.groupby('nuclide')['value'].max()
    small_nuc = max_rel_by_nuc[max_rel_by_nuc < threshold].index
    large_nuc = max_rel_by_nuc[max_rel_by_nuc >= threshold].index

    large_adf = adfc[adfc['nuclide'].isin(large_nuc)]
    small_adf = adfc[adfc['nuclide'].isin(small_nuc)]

    if small_adf.empty:
        agg = large_adf
    else:
        other_rows = (small_adf.groupby('time').agg({
            'value'         :        'sum',
            'run_lbl'       :      'first',
            'block'         :      'first',
            'block_num'     :      'first',
            'variable'      :      'first',
            'var_unit'      :      'first',
            'time_unit'     :      'first'
            }).reset_index()
        )
        other_rows['nuclide'] = 'Other'
        agg = pd.concat([large_adf, other_rows], ignore_index=True)

    return agg

###########################################

class FileParser:

    def __init__(self, filepath, run_lbl, time_unit):
        self.filepath = filepath
        self.run_lbl = run_lbl
        self.time_unit = time_unit
        self.results = ALARADFrame()

    # ---------- Parsing Boolean Logic Functions ----------

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

    @staticmethod
    def _try_float(thalf):
        '''
        Convert a numeric half-life value to a float. For stable nuclides or
            the "total" row, leave "half-life" value as is.

        Arguments:
            thalf (str): Half-life value for unstable nuclides, "stable" for
                stable nuclides, "None" for "total" row.

        Returns:
            thalf (float or str): Converted half-life value for numeric
                strings.
        '''

        try:
            return float(thalf)

        except ValueError:
            return thalf

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
                    'time' : Cooling time of data entry,
                    'time_unit' : Units for cooling times,
                    'nuclide' : Individual nuclide,
                    'half_life' : Half-life of unstable nuclides or "stable",
                    'run_lbl' : Distinguisher between runs,
                    'block' : ALARADFrame block integer enumerator,
                    'block_num' : Geometric position of block,
                    'variable' : ALARADFrame variable integer enumerator,
                    'var_unit' : Unit for corresponding variable,
                    'value' : Float value for the corresponding variable
                } 
        '''

        header_line = current_table_lines[0].strip()
        data_lines = current_table_lines[1:]

        raw_cols = header_line.split()
        nuclide_col = raw_cols[0]
        thalf_col = raw_cols[1]
        times_w_units = raw_cols[2:]
        converted_times = extract_time_vals(
            times_w_units, to_unit=self.time_unit
        )
        block_type, block_num_and_name = current_block.split(' #')
        block_num, _, block_name = block_num_and_name.partition(' ')
        variable, unit = current_parameter.split(' [')
        
        reader = DictReader(
            [' '.join(line.split()) for line in data_lines],
            fieldnames=(
                [nuclide_col, thalf_col] + [str(t) for t in converted_times]
            ),
            delimiter=' ',
            skipinitialspace=True
        )

        return [{
            'time'          :                                    time,
            'time_unit'     :                          self.time_unit,
            'nuclide'       :                        row[nuclide_col],
            'half_life'     :         self._try_float(row[thalf_col]),
            'run_lbl'       :                            self.run_lbl,
            'block'         :      ALARADFrame.BLOCK_ENUM[block_type],
            'block_name'    :                              block_name,
            'block_num'     :            int(block_num.split(':')[0]),
            'variable'      :     ALARADFrame.VARIABLE_ENUM[variable],
            'var_unit'      :                      unit.split(']')[0],
            'value'         :                   float(row[str(time)])
        } for row in reader for time in converted_times]

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
                current_table_lines = [line]
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


class FispactParser:

    @staticmethod
    def fispact_to_adf(run_lbl, output_path):
        '''
        Parse output data from a FISPACT-II output file and write out relevant
            data in the canonical ALARADFrame data structure, to be integrated
            in a single frame alongside ALARA output data.

        Arguments:
            run_lbl (str): Distinguisher between runs (i.e. "fispact-ii").
            output_path (str or pathlib._local.PosixPath): Path to the
                FISPACT-II output file. Must have the file suffix ".fis".
        
        Returns:
            adf (alara_output_processing.ALARADFrame): Specialized DataFrame
                for ALARA output data, containing FISPACT-II output data as
                well.
            all_nucs (set): Set of all nuclides catalogued in the inventory
                across all times.
        '''

        with pypact.Reader(output_path) as output:
            rows = []
            time_zero = 0
            all_nucs = set()
            row = {k: -1 for k in ['block', 'block_name', 'block_num']}
            row['run_lbl'] = run_lbl
            row['time_unit'] = 's'
            for time in output.inventory_data:
                
                # FISPACT-II marks both pre-irradiation and shutdown as
                # time.cooling_time = 0. This conditional sets the first
                # instance (pre-irradiation) to -1 to match the ALARADFrame
                # time label convention.
                if time.cooling_time == 0 and time_zero == 0:
                    time.cooling_time = -1
                    time_zero += 1

                row['time'] = time.cooling_time
                mass = sum(nuc.grams for nuc in time.nuclides) / 1e3 # [kg]
                for n in time.nuclides:
                    nuclide = f'{n.element.lower()}-{n.isotope}{n.state}'
                    all_nucs.update({nuclide})
                    half_life = n.half_life if n.half_life != 0 else -1
                    row['nuclide'] = nuclide
                    row['half_life'] = half_life

                    # Standard units from FISPACT-II User Manual, Table 7
                    # https://fispact.ukaea.uk/_documentation/UKAEA-R18001.pdf
                    for name, enum in ALARADFrame.VARIABLE_ENUM.items():
                        match name:
                            case 'Number Density':
                                value = n.atoms / mass
                                unit  = 'atoms/kg'
                            case 'Specific Activity':
                                value = n.activity / mass
                                unit  = 'Bq/kg'
                            case 'Total Decay Heat':
                                value = (n.heat * 1e3) / mass
                                unit  = 'W/kg' # kW/kg -> W/kg
                            case 'Alpha Heat':
                                value = (n.alpha_heat * 1e3) / mass
                                unit  = 'W/kg' # kW/kg -> W/kg
                            case 'Beta Heat':
                                value = (n.beta_heat * 1e3) / mass
                                unit  = 'W/kg' # kW/kg -> W/kg
                            case 'Gamma Heat':
                                value = (n.gamma_heat * 1e3) / mass
                                unit  = 'W/kg' # kW/kg -> W/kg
                            case 'Contact Dose':
                                value = n.dose
                                unit  = 'Sv/hr'

                        row['variable'] = enum
                        row['var_unit'] = unit
                        row['value'] = value
                        rows.append(row.copy())

        return ALARADFrame(rows).create_total_rows(), all_nucs


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
    CANONICAL_COLUMN_ORDER = [
        'time', 'time_unit', 'nuclide', 'half_life', 'run_lbl',
        'block', 'block_num', 'variable', 'var_unit', 'value'
    ]

    @property
    def _constructor(self):
        return ALARADFrame

    def _single_element_all_nuclides(self, element):
        '''
        Create a set of all nuclides of a given element in an ALARADFrame to
            be called within filter_rows().
        
        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
            elements (str): Single element for which to find all nuclides in
                self.

        Returns:
            nuclides (set): Set of all nuclides of the selected element
                present in the ALARADFrame.
        '''

        return set(self.loc[
            self['nuclide'].str.startswith(f'{element.lower()}-', na=False),
            'nuclide'
        ])
    
    def _get_initial_nuclides(self):
        '''
        Create a set of all nuclides in initial, pre-irradiated material
            mixtures in an ALARADFrame to be called within filter_rows().

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.

        Returns:
            initial_nucs (set): Set of all nuclides contained in the pre-
                irradiated material mixture(s).
        '''

        return set(self.loc[
            (self['time'] == -1)
            & (self['value'] > 0)
            & (self['nuclide'] != 'total'),
            'nuclide'
        ])
    
    @staticmethod
    def _select_radionucs(filters):
        '''
        Boolean function to determine whether or not a list of filters in the
            optional "half_life" key of filter_dicts is seeking unstable
            nuclides.
        
        Arguments:
            filters (list): List of filters provided for the "half_life" key.
        
        Returns:
            select_radionucs (bool): True if the 0th element of 
                filter_dict["half_life"] = "radioactive", "unstable", or
                either of the Python less than/greater than operators (">",
                "<"), otherwise False.
        '''

        return filters[0] in {'radioactive', 'unstable'} | set(OPS)
    
    @staticmethod
    def _filter_numerically(expression, numeric_set):
        '''
        Create a subset of a pre-existing set of contained that match the
            mathematical operation and threshold provided in the partial
            expression provided (i.e. all half-lives
            greater than 1e6 seconds).

        Arguments:
            expression (array-like): Array-like collection of a partial
                mathematical expression of the form:
                    expression[0] (str): String of a Python-formatted
                    less than/greater than operator (">", "<").
                    expression[1] (int or float): threshold comparison value.
                Together, the expression would be of the form ['>', 1e6].
            numeric_set (set): Set of all numeric values to be filtered.

        Returns:
            numeric_subset (set): Subset of input set containing only values
                that match the supplied mathematical expression.
        '''

        op, threshold = expression
        return {x for x in numeric_set if OPS[op](x, threshold)}
    
    def _get_half_lives(self, filters):
        '''
        Create a set of all numeric half-lives in an ALARADFrame.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame.
        
        Returns:
            half_lives (set): Set of all numeric half-lives in the ALARADFrame
                (i.e. excluding "stable" for stable nuclides).
        '''

        half_lives = set(self.loc[self['half_life'] > 0, 'half_life'])

        # If filtered_dict contains half-life threshold operations,
        # find the subset of half-lives matching the conditions
        if filters[0] in OPS and isinstance(filters[1], (int, float)):
            half_lives &= self._filter_numerically(filters, half_lives)

        return half_lives

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
            if col_name not in filtered_adf.columns:
                warn(f'Column "{col_name}" not found. Skipping.')
                continue
            
            if not isinstance(filters, list):
                filters = [filters]

            if col_name == 'time' and filters[0] in OPS:
                filters = filtered_adf._filter_numerically(
                    filters, set(filtered_adf['time'])
                )

            if col_name == 'nuclide':
                nuclides = set()
                initial_nucs = filtered_adf._get_initial_nuclides()

                if filters[0] == 'initial':
                    nuclides.update(initial_nucs)

                elif filters[0] == 'transmuted':
                    all_nucs = set(filtered_adf['nuclide'])
                    nuclides.update(all_nucs - initial_nucs - {'total'})

                else:
                    for f in filters:
                        nuclides.update(
                            filtered_adf._single_element_all_nuclides(f)
                            if ('-' not in f and f.lower() != 'total')
                            else [f]
                        )

                filters = nuclides

            if col_name == 'half_life':
                if self._select_radionucs(filters):
                    filters = filtered_adf._get_half_lives(filters)
                
                elif filters[0] == 'stable':
                    filters[0] = -1

            filtered_adf = filtered_adf[filtered_adf[col_name].isin(filters)]

        if filtered_adf.empty:
            warn('Excessive filtering. No matching rows for given filters.')    

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

        totals = self.filter_rows({
            'nuclide' : 'total'
            })[['time', 'value']].to_numpy()
        total_map = {time: value for time, value in totals}
        adf_rel = self[self['nuclide'] != 'total'].copy()

        adf_rel['value'] = adf_rel.apply(
            lambda row: row['value'] / total_map[row['time']],
            axis=1
        )
        adf_rel['var_unit'] = [None] * len(adf_rel)

        return adf_rel

    #### FISPACT-II Hybrid ALARADFrame Operations ####
    ####    (ALARADFrames with FISPACT-II data)   ####

    def create_total_rows(self):
        '''
        Calculate and insert "total" nuclide row data for each variable,
            time-step in an ALARADFrame. Only used when loading FISPACT-II
            output data into an ALARADFrame, as ALARA output data always
            contains cumulative "total" data at each time-step.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized DataFrame
                for ALARA output data, containing FISPACT-II output data as
                well.

        Returns:
            adf_with_totals (alara_output_processing.ALARADFrame): Updated
                ALARADFrame with calculated totals.
        '''

        totals = self.groupby(
            ['run_lbl', 'variable', 'time'], as_index=False
        ).agg({
            'value'      :   'sum',
            'time_unit'  : 'first',
            'var_unit'   : 'first',
            'block'      : 'first',
            'block_name' : 'first',
            'block_num'  : 'first',
        })

        totals['nuclide'] = 'total'
        totals['half_life'] = 0

        return pd.concat([self, totals], ignore_index=True)

    def fill_skipped_nucs(self, all_nucs):
        '''
        Fill in rows with response values of 0.0 for missing nuclides at
            different times. Only necessary once after loading for
            ALARADFrames containing FISPACT-II data, as FISPACT-II output
            files ignore nuclides not present at a given time if their number
            densities are 0, even if they are present at other times. Not
            necessary for ALARA-only data because ALARA tracks all nuclides 
            present at any time for each time, marking appropriate responses
            of 0 in such cases already.

        Arguments:
            self (alara_output_processing.ALARADFrame): Specialized DataFrame
                for ALARA output data, containing FISPACT-II output data as
                well.
            all_nucs (set): Set of all nuclides catalogued in the inventory
                across all times.

        Returns:
            filled_adf (alara_output_processing.ALARADFrame): Expanded
                ALARADFrame containing response data for all nuclides at all
                times, including zeros.
        '''

        fill_rows = []
        row = {k: self[k].iat[0] for k in ['run_lbl', 'block', 'block_name']}
        for time in self['time'].unique():
            time_filtered = self.filter_rows({'time' : time})
            row['time'] = time
            row['time_unit'] = time_filtered['time_unit'].iat[0]
            for nuc in (all_nucs - set(time_filtered['nuclide'])):
                row['nuclide'] = nuc
                row['half_life'] = self.filter_rows({
                    'nuclide' : nuc
                })['half_life'].iat[0]
                for var in self['variable'].unique():
                    row['variable'] = var
                    row['var_unit'] = self.filter_rows({
                        'variable' : var
                    })['var_unit'].iat[0]
                    row['value'] = 0.0
                    fill_rows.append(row.copy())
        
        return pd.concat([self, ALARADFrame(fill_rows)], ignore_index=True)


class DataLibrary:

    def __init__(self):
         self.adf = None

    def make_entries(self, runs_dict, time_unit='s'):
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

                Note: To read in a FISPACT-II output table, its path suffix in
                runs_dict must be ".fis". 
                
        Returns:
            self.adf (alara_output_processing.ALARADFrame): Specialized ALARA
                output DataFrame containing combined data from all tables in 
                all runs contained within runs_dict.
        '''

        dfs = []
        for run_lbl, output_path in runs_dict.items():
            if Path(output_path).suffix == '.fis':
                missing_nucs, all_nucs = FispactParser.fispact_to_adf(
                    run_lbl, output_path
                )
                data = missing_nucs.fill_skipped_nucs(all_nucs)[
                    ALARADFrame.CANONICAL_COLUMN_ORDER
                ]
        
            else:
                parser = FileParser(
                    output_path, run_lbl=run_lbl, time_unit=time_unit
                )
                data = parser.extract_tables()
            
            dfs.append(data)

        self.adf = ALARADFrame(pd.concat(dfs).fillna(0.0))

        return self.adf

###########################################

def args():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--filepath', '-f', required=True
    )
    argparser.add_argument(
        '--run_label', '-r', required=False
    )
    argparser.add_argument(
        '--time_unit', '-t', required=False
    )
    return argparser.parse_args()

def main():
    alara_data = FileParser(
        args().filepath, args().run_label, args().time_unit
    )
    adf = alara_data.extract_tables()
    adf.to_csv(f'{args().run_label}.csv')

if __name__ == '__main__':
    main()