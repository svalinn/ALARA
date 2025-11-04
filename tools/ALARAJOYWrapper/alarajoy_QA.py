import subprocess
import pandas as pd
import matplotlib.pyplot as plt
from string import Template
from pathlib import Path
import sys
sys.path.append('..')
from alara_output_parser import parse_tables

#--------------- Running Single Parent Element Simulation(s) -----------------

INPUT = 'alara.inp'

# Adapted from ALARA/examples/singleElement.ala
alara_input = Template(
'''
geometry rectangular
dimension	x
		0.0
        1       5.0
end
mat_loading
        inner_zone1  mix1
end
material_lib ../../data/matlib.sample
element_lib ../../data/nuclib.std
data_library alaralib $datalib
mixture mix1
        element $element              1.0     1.00
end
flux flux_1 ../../examples/ref_flux_files/fluxfnsfIBfw_518MW.txt  1.0   0   default
schedule 2_year
	2 y  flux_1  steady_state  0 s
end
pulsehistory steady_state
	1	0 s
end
dump_file dump_singleElement
cooling
	1e-5 y
	1e-2 y
	1 y
	100 y
	10000 y
end
output interval
        units Bq kg
        number_density
        specific_activity
	total_heat
	dose contact $datalib ../../data/ANS6_4_3
end
## 
truncation  1e-7
'''
)

def fill_alara_template(element, datalib):
    '''
    Substitute in the specific single parent element and path to a
        pre-converted ALARA binary library, such as that for either FENDL2 or
        ALARAJOY-processed FENDL3, to a template containing a generalized
        ALARA input file text for a simple single parent element simulation.
    Arguments:
        element (str): Single parent element to be irradiated.
        datalib (str): Path to the binary library.
    
    Returns:
        alara_input (str): String template with appropriate variables
            substituted in for Template identifiers.
    '''

    return alara_input.substitute(element=element, datalib=datalib)

def write_alara_input_file(template):
    '''
    Write out the ALARA input card from the prefilled template.
    Arguments:
        template (str): String template with appropriate variables substituted
            in for Template identifiers.
    Returns:
        None
    '''

    with open(INPUT, 'w') as f:
        f.write(template)

def run_alara(element, libname):
    '''
    Invoke subprocess.run() to run ALARA for the single parent element
        irradiation simulation. Specify destination for ALARA tree file and
        capture stdout to an output file to be read by
        alara_pandas_parser.parse_tables().
    Arguments:
        element (str): Single parent element to be irradiated.
        libname (str): Name of the source data library (i.e. fendl2, fendl3,
            etc.)
    Returns:
        output (str): Path to the ALARA redirected ALARA stdout formatted as a
            text file.
    '''

    filename_base = f'{element}_{libname}'
    output = f'{filename_base}.out'
    Path(output).unlink(missing_ok=True)
    with open(output, 'w') as outfile:
        subprocess.run(
            ['alara', '-t', f'{filename_base}.tree', '-v', '3', INPUT],
            stdout=outfile,
            stderr=subprocess.STDOUT,
            check=True
        )

    return output

#---------- Loading Results of Simulation(s) into a Data Structure -----------

def make_entry(datalib, variable, unit, data):
    '''
    Construct a dictionary entry for a single Pandas DataFrame and its
        associated metadata for updating into a larger metadata dictionary.
    Arguments:
        datalib (str): Data source for the DataFrame (i.e. fendl2,
            ALARAJOY-fendl3, etc.).
        variable (str): Dependent variable evaluated in DataFrame (i.e. Number
            Density, Specific Activity, etc.).
        unit (str): Associated units for the above variable (i.e. atoms/kg, 
            Bq/kg, etc.).
        data (pandas.core.frame.DataFrame): Pandas DataFrame described by
            the above metadata.
    Returns:
        entry (dict): Single data entry for dictionary containing potentially
            multiple dataframes and associated metadata.
    '''

    return {
        f'{datalib} {variable}': {
            'Data Source': datalib,
            'Variable': variable,
            'Unit': unit,
            'Data': data
        }
    }

def process_metadata(
        data_source,
        inp_datalib=None,
        inp_variable=None,
        inp_unit=None
        ):
    '''
    Flexibly create a dictionary of subdictionaries containing Pandas
        DataFrames with associated metadata containing ALARA output data for
        different variables. Allows for processing of either existing
        DataFrames, with the requirement that the user provide the relevant
        data source, evaulated variable, and its relevant units to be packaged
        into a metadata dictionary. Alternatively, this function can directly
        read in an ALARA output file and parse all tables and their metadata
        internally.
    Arguments:
        data_source (dict or pandas.core.frame.DataFrame): ALARA output data.
            If parsing directly from ALARA output files, data_source must be
            formatted as a dictionary of the form:
            data_source = {
                Data Library 1 : path/to/output/file/for/data/library1,
                Data Library 2 : path/to/output/file/for/data/library2
            }
            If processing a preexisting DataFrame, then data_source is just
            the DataFrame itself.
        inp_datalib (str or None, optional): Data source of the selected
            DataFrame. Required if processing a preexisting DataFrame;
            irrelevant if parsing directly from ALARA output files.
            (Defaults to None)
        inp_variable (str or None, optional): Evaluated variable for the
            selected DataFrame. Required if processing a preexisting
            DataFrame; irrelevant if parsing directly from ALARA output files.
            (Defaults to None)
        inp_unit (str or None, optional): Appropriate unit for the evaluated
            variable for the selected DataFrame. Required if processing a
            preexisting DataFrame; irrelevant if parsing directly from ALARA
            output files.
            (Defaults to None)
    Returns:
        dfs (list of dicts): List of dictionaries containing ALARA output
            DataFrames and their metadata, of the form:
            df_dict = {
                'Data Source' : (Either 'fendl2' or 'fendl3'),
                'Variable'    : (Any ALARA output variable, dependent on ALARA
                                 run parameters),
                'Unit'        : (Respective unit matching the above variable),
                'Data'        : (DataFrame containing ALARA output data for
                                 the given data source, variable, and unit)
            }
    '''

    dfs = {}

    if type(data_source) == pd.core.frame.DataFrame:
        dfs.update(
            make_entry(inp_datalib, inp_variable, inp_unit, data_source)
        )
        return dfs

    for datalib, output_path in data_source.items():
        output_tables = parse_tables(output_path)
        for key, data in output_tables.items():
            variable_w_unit, _ = key.split(' - ')
            variable, unit = variable_w_unit.split(' [')
            dfs.update(make_entry(datalib, variable, unit.strip(']'), data))

    return dfs

#------------------------- Plotting Helper Functions -------------------------

def process_time_vals(alara_df, seconds=True):
    '''
    Convert the cooling times of the ALARA analysis post-shutdown to seconds.
    Arguments:
        alara_df (pandas.core.frame.DataFrame): DataFrame containing the
            extracted tabular data for a single variable and interval/zone of
            an ALARA run.
        seconds (bool, optional): Option to convert cooling times from years
            to seconds.
            (Defaults to True)
    Returns:
        times (list): List of the ALARA cooling times, converted to seconds
    '''

    times = []
    time_dict = {'shutdown' : 0.0}
    time_dict['y'] = 365*24*60*60 if seconds else 1

    for column in alara_df.columns[1:]:
        if column == 'shutdown':
            times.append(time_dict['shutdown'])
        else:
            time = column.split('y')[0]
            times.append(float(time) * time_dict['y'])

    return times

def separate_total(alara_df):
    '''
    Cut the "total" row from the ALARA ouptut DataFrame into its own 
        DataFrame.
    Arguments:
        alara_df (pandas.core.frame.DataFrame): DataFrame containing the
            extracted tabular data for a single variable and interval/zone of
            an ALARA run.
    Returns:
        alara_df_isotopes (pandas.core.frame.DataFrame): DataFrame containing
            all of the radioisotope data for a given variable.
        alara_df_total (pandas.core.frame.DataFrame): DataFrame containing one
            row for the total data.
    '''
    if alara_df.columns[0] == 'interval':
        alara_df.at[0, 'interval'] = 'total'
        return pd.DataFrame(), alara_df

    else:
        mask_total = (alara_df['isotope'] == 'total')
        alara_df_total = alara_df[mask_total].copy()
        alara_df_isotopes = alara_df.drop(alara_df[mask_total].index)

        return alara_df_isotopes, alara_df_total

def aggregate_small_percentages(df, relative=False, threshold=0.05):
    '''
    Consolidate all rows in a DataFrame that do not have any cells with a 
        contribution of more than a threshold value to the total for its
        respective column. Rows that do not have any cells that surpass its
        column's threshold are aggregated into a new "Other" row. If a row has
        at least one column value above the threshold, then the whole row is
        preserved.
    Arguments:
        df (pandas.core.frame.DataFrame): DataFrame containing the extracted
            tabular data for a single variable and interval/zone of an ALARA
            run.
        relative (bool, optional): Option for DataFrames already processed
            by relative_contributions().
            (Defaults to False)     
        threshold (float or int): Proportional threshold value for inclusion
            cutoff.
            (Defaults to 0.05)
    Returns:
        df (pandas.core.frame.DataFrame): Processed DataFrame (potentially)
            with new row, "Other", containing all aggregrated data below the
            thresholds.
    '''

    cols = df.columns[1:]

    if relative:
        rel_adjustment = [1] * len(cols)
    else:
        df, df_total = separate_total(df)
        rel_adjustment = [float(df_total[col].iloc[0])
                          for col in cols]

    threshold_vals = {
        col: threshold * adj for col, adj in zip(cols, rel_adjustment)
    }

    small_mask = (df[cols] < pd.Series(threshold_vals)).all(axis=1)
    other_row = pd.Series(0.0, index=df.columns, dtype=object)
    other_row[df.columns[0]] = 'Other'
    other_row[cols] = df.loc[small_mask, cols].sum() 

    df = df.loc[~small_mask].reset_index(drop=True)
    if other_row[cols].sum() > 0:
        df = pd.concat([df, pd.DataFrame([other_row])], ignore_index=True)

    return df

def specify_data(
        df_dict,
        total=True, 
        element='', 
        sort_by_time='shutdown', 
        head=None,
        relative = False,
        filter_small=False,
        threshold=0.05,
        seconds=True
        ):
    '''
    Select or order a DataFrame containing ALARA output data for a partiuclar
        variable according to user needs.
    
    Arguments:
        df_dict (dict): Dictionary containing an ALARA output DataFrame and
            its metadata, of the form:
            df_dict = {
                'Data Source' : (Either 'fendl2' or 'fendl3'),
                'Variable'    : (Any ALARA output variable, dependent on ALARA
                                 run parameters),
                'Unit'        : (Respective unit matching the above variable),
                'Data'        : (DataFrame containing ALARA output data for
                                 the given data source, variable, and unit)
            }
        total (bool, optional): Option to separate the "total" row from the
            provided DataFrame.
            (Defaults to True)
        element (str or list, optional): Option to plot only the isotopes of a
            single element or list of selected elements. If left blank, all
            elements in the original DataFrame will remain present.
            (Defaults to '')
        sort_by_time (str, optional): Option to sort the DataFrame by the data
            in a particular time column.
            (Defaults to 'shutdown')
        head (int or None, optional): Option to truncate the DataFrame to a
            particular number of rows.
            (Defaults to None)
        filter_small (bool, optional): Option to internally call
            aggregate_small_percentages() to aggregate all data below a chosen
            threshold to a new 'Other' row.
            (Defaults to False)
        threshold (float or int): Proportional threshold value for inclusion
            cutoff for aggregrate_small_percentages().
            (Defaults to 0.05)      
    Returns:
        df (pandas.core.frame.DataFrame): Processed DataFrame based on user
            input.
        times (list): List of the column header times in seconds.
    '''

    df = df_dict['Data']
    times = process_time_vals(df, seconds)

    if filter_small:
        df = aggregate_small_percentages(df, relative, threshold)

    if not total:
        df, df_total = separate_total(df)

    if element:
        if type(element) is not list:
            element = [element]
        regex = '|'.join(fr'{el}-' for el in element)
        df = df[df['isotope'].str.contains(regex, case=False, na=False)]

    if sort_by_time and head:
        df = df.sort_values(sort_by_time, ascending=False).head(head)

    return df, times

def split_label(label):
    '''
    Split the string of a series' label to extract the isotope being plotted
        and (conditionally) the data source attached to it. The data source
        will be present parenthetically if two data sources are being plotted
        comparitavely.
    Arguments:
        label (str): Series label generated from 
            ax.get_legend_handles_labels().
    
    Returns:
        isotope (str): Isotope being plotted.
        datalib (str): Data source of plotted isotope. Will be empty string
            for single data source plotting.
    '''

    if '(' in label:
        parts = label.split('(')
        isotope = parts[0].strip()
        datalib = f'({parts[1].strip(')')})'
    else:
        isotope = label.strip()
        datalib = ''
    return isotope, datalib

def reformat_isotope(isotope):
    '''
    Restructure the string describing an isotope to capitalize its first
        letter and place the atomic number in a superscript for cleaner
        presentation in legend. Skips "isotope" entries of "total" or "Other".
    Arguments:
        isotope (str): Identifier of the isotope of the form element-A.
    
    Returns:
        isotope (str): Reformatted identifier of the isotope of the form
            ᴬelement.
    '''

    if isotope == 'total' or isotope == 'Other':
        return isotope
    else:
        element, A = isotope.split('-')
        element = element.capitalize()
        return f'$^{{{A}}}\\mathrm{{{element}}}$'

def construct_legend(ax, data_comp=False):
    '''
    Create a custom pyplot legend that exists outside of the grid itself and
        can group like-isotopes together from compared data sets for clarity.
    
    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib axis object of the plot
            being constructed.
        data_comp (bool, optional): Boolean setting for comparison between two
            data sets.
            (Defaults to False)
    
    Returns:
        None
    '''

    handles, labels = ax.get_legend_handles_labels()
    labels_sorted_with_handles = sorted(
        zip(labels, handles),
        key=lambda x: (
            split_label(x[0]) if data_comp else (split_label(x[0])[0], x[0])
        )
    )

    grouped_handles = []
    grouped_labels = []
    prev_isotope = None

    for lbl, h in labels_sorted_with_handles:
        isotope, datalib = split_label(lbl)
        isotope = reformat_isotope(isotope)

        if prev_isotope is not None and isotope != prev_isotope:
            grouped_handles.append(plt.Line2D([], [], linestyle=''))
            grouped_labels.append('――――――')
        grouped_handles.append(h)
        grouped_labels.append(f'{isotope} {datalib}')
        prev_isotope = isotope

    ax.legend(
        grouped_handles,
        grouped_labels,
        loc='center left',
        bbox_to_anchor=(1.025, 0.5),
        borderaxespad=0.,
        fontsize='small',
        handlelength=1.5,
        handletextpad=0.5,
    )

#---------------------------- Plotting functions -----------------------------

def plot_single_response(
        df_dicts, 
        total=False, 
        element='', 
        sort_by_time='shutdown', 
        head=None,
        yscale='log',        
        relative=False,
        filter_small=False,
        threshold=0.05,
        seconds=True
        ):
    '''
    Create a simple x-y plot of a given variable tracked in an ALARA output
        table (as stored in a DataFrame) against a log timescale. Options for
        plotting a single data source, as well as two data sources against
        each other. Plot will contain unique lines for the isotopes 
        represented in the data, with options to show only certain elements
        and/or only the largest contributors at a given cooling time.
        Additionally, the cumulative total values across all isotopes can be
        plotted separately from individual isotopic data using the combination
        of the parameters: total=True, head=1.
    Arguments:
        df_dicts (dict or list): Single dictionary containing an ALARA output 
            DataFrame and its metadata, of the form:
            df_dict = {
                'Data Source' : (Either 'fendl2' or 'fendl3'),
                'Variable'    : (Any ALARA output variable, dependent on ALARA
                                 run parameters),
                'Unit'        : (Respective unit matching the above variable),
                'Data'        : (DataFrame containing ALARA output data for
                                 the given data source, variable, and unit)
            }
            Alternatively, if comparing two data sources, df_dicts can be a
            list of dictionaries of the above form.
        total (bool, optional): Option to include the cumulative total
            contribution from all isotopes towards the select variable in the
            plot. If total=True, the total array will be treated equivalently
            to any of the other isotopes and will be plotted alongside them.
            If total=True and head=1, only the total will be plotted.
            (Defaults to False)
        element (str or list, optional): Option to plot only the isotopes of a
            single element or list of selected elements. If left blank, all
            elements in the original DataFrame will remain present.
            (Defaults to '')
        sort_by_time (str, optional): Option to sort the DataFrame by the data
            in a particular time column.
            (Defaults to 'shutdown')
        head (int or None, optional): Option to truncate the DataFrame to a
            particular number of rows.
            (Defaults to None)
        relative (bool, optional): Option to plot relative values with respect
            to totals at each cooling time.
            (Defaults to False)
        filter_small (bool, optional): Option to internally call
            aggregate_small_percentages() to aggregate all data below a chosen
            threshold to a new 'Other' row.
            (Defaults to False)
        threshold (float or int): Proportional threshold value for inclusion
            cutoff for aggregrate_small_percentages().
            (Defaults to 0.05)    
    Returns:
        None    
    '''

    data_comp = True
    fig, ax = plt.subplots(figsize=(10,6))

    # Single data source -- Data provided as dict or DataFrame
    if type(df_dicts) is not list:
        df_dicts = [df_dicts]
        data_comp = False

    # Preprocess data to user specifications
    all_labels = set()
    all_data = []
    for df_dict in df_dicts:
        df, times = specify_data(
            df_dict, 
            total=total, 
            element=element, 
            sort_by_time=sort_by_time, 
            head=head,
            relative=relative,
            filter_small=filter_small,
            threshold=threshold,
            seconds=seconds
        )

        df = df.T
        for col in df.columns:
            label_text = f"{df[col].iloc[0]}"
            all_labels.add(label_text)

        all_data.append((df_dict, df, times))

    labels_sorted = sorted(all_labels)

    cmap = plt.cm.get_cmap('Dark2')
    color_map = {lbl: cmap(i % cmap.N) for i, lbl in enumerate(labels_sorted)}

    line_styles = ['-', ':']

    # Plot data
    for i, (df_dict, df, times) in enumerate(all_data):
        linestyle = line_styles[i % len(line_styles)]
        for col in df.columns:
            label_text = f"{df[col].iloc[0]}"
            color = color_map[label_text]
            label = label_text
            if data_comp:
                label = f"{label_text} ({df_dict['Data Source']})"

            ax.plot(
                times,
                list(df[col])[1:],
                label=label,
                color=color,
                linestyle=linestyle,
            )

    # Titles and labels, according to user specifications
    title_suffix = (f'{df_dicts[0]['Variable']} vs Cooling Time ')

    if relative:
        title_suffix += 'Relative to Total at Each Cooling Time '

    if head:
        title_suffix += (
            f'\n(DataFrame Head Sorted by Values at {sort_by_time})'
            )

    if data_comp:
        title_prefix = (
            f'{df_dicts[0]['Data Source']}, {df_dicts[1]['Data Source']} '
            'Comparison: \n'
        )
    else:
        title_prefix = f'{df_dict['Data Source']}: '

    ax.set_title(title_prefix + title_suffix)
    if not relative:
        ax.set_ylabel(f'{df_dict['Variable']} [{df_dict['Unit']}]')
    ax.set_xlabel(f'Time ({'s' if seconds else 'y'})')
    ax.set_xscale('log')
    ax.set_yscale(yscale)

    construct_legend(ax, data_comp)

    ax.grid(True)
    plt.tight_layout(rect=[0, 0, 0.85, 1])
    plt.show()