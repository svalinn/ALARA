import subprocess
from string import Template
from pathlib import Path
import matplotlib.pyplot as plt

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

#------------------------ Plotting Helper Functions --------------------------

def split_label(label):
    '''
    Split the string of a series' label to extract the isotope being plotted
        and (conditionally) the run label attached to it. The run parameter
        will be present parenthetically if two runs are being plotted
        comparitavely.
    Arguments:
        label (str): Series label generated from 
            ax.get_legend_handles_labels().
    
    Returns:
        isotope (str): Isotope being plotted.
        run_lbl (str): Run label of plotted isotope. Will be empty string
            for single run plotting.
    '''

    if '(' in label:
        parts = label.split('(')
        isotope = parts[0].strip()
        run_lbl = f'({parts[1].strip(')')})'
    else:
        isotope = label.strip()
        run_lbl = ''
    return isotope, run_lbl

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
        data_key = 'Data',
        sort_by_time='shutdown', 
        head=None,
        total=False,
        yscale='log',        
        relative=False,
        seconds=True
        ):
    '''
    Create a simple x-y plot of a given variable tracked in an ALARA output
        table (as stored in an ALARADFrame) against a log timescale. Options
        for plotting a single run, as well as two runs against each other.
        Plot will contain unique lines for the isotopes represented in the
        data, with options to show only certain elements and/or only the
        largest contributors at a given cooling time. Additionally, the
        cumulative total values across all isotopes can be plotted separately
        from individual isotopic data using the combination of the parameters:
        total=True, head=1.

    Arguments:
        df_dicts (dict or list): Single dictionary containing an ALARA output 
            DataFrame and its metadata, of the form:
            df_dict = {
                'Run Label'   : (Distinguisher between runs),
                'Variable'    : (Any ALARA output variable, dependent on ALARA
                                 run parameters),
                'Unit'        : (Respective unit matching the above variable),
                'Data'        : (ALARADFrame containing ALARA output data for
                                 the given run parameter, variable, and unit)
            }
            Alternatively, if comparing two ALARA runs, df_dicts can be a list
            of dictionaries of the above form.
        total (bool, optional): Option to include the cumulative total
            contribution from all isotopes towards the select variable in the
            plot. If total=True, the total array will be treated equivalently
            to any of the other isotopes and will be plotted alongside them.
            If total=True and head=1, only the total will be plotted.
            (Defaults to False)
        element (str or list, optional): Option to plot only the isotopes of a
            single element or list of selected elements. If left blank, all
            elements in the original ALARADFrame will remain present.
            (Defaults to '')
        sort_by_time (str, optional): Option to sort the ALARADFrame by the
            data in a particular time column.
            (Defaults to 'shutdown')
        head (int or None, optional): Option to truncate the ALARADFrame to a
            particular number of rows.
            (Defaults to None)
        relative (bool, optional): Option to plot relative values with respect
            to totals at each cooling time.
            (Defaults to False)
        seconds (bool, optional): Option to convert cooling times from
            years to seconds.
            (Defaults to True)
            
    Returns:
        None    
    '''

    data_comp = True
    fig, ax = plt.subplots(figsize=(10,6))

    # Single run -- Data provided as dict or ALARADFrame
    if not isinstance(df_dicts, list):
        df_dicts = [df_dicts]
        data_comp = False

    # Preprocess data to user specifications
    all_labels = set()
    all_data = []
    for df_dict in df_dicts:
        adf = df_dict[data_key]
        times = adf.process_time_vals(seconds=seconds)
        adf = adf.T
        for col in adf.columns:
            label_text = f"{adf[col].iloc[0]}"
            all_labels.add(label_text)

        all_data.append((df_dict, adf, times))

    labels_sorted = sorted(all_labels)

    cmap = plt.cm.get_cmap('Dark2')
    color_map = {lbl: cmap(i % cmap.N) for i, lbl in enumerate(labels_sorted)}

    line_styles = ['-', ':']

    # Plot data
    for i, (df_dict, adf, times) in enumerate(all_data):
        linestyle = line_styles[i % len(line_styles)]
        for col in adf.columns:
            label_text = f"{adf[col].iloc[0]}"
            if not total and label_text == 'total':
                continue
            color = color_map[label_text]
            label = label_text
            if data_comp:
                label = f"{label_text} ({df_dict['Run Label']})"

            ax.plot(
                times,
                list(adf[col])[1:],
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
            f'\n(ALARADFrame Head Sorted by Values at {sort_by_time})'
            )

    if data_comp:
        title_prefix = (
            f'{df_dicts[0]['Run Label']}, {df_dicts[1]['Run Label']} '
            'Comparison: \n'
        )
    else:
        title_prefix = f'{df_dict['Run Label']}: '

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