import sys
import pandas as pd
sys.path.append('../../../alara_output_processing')
sys.path.append('../..')
import alara_output_processing as aop
from njoy_tools import elements
from string import Template
import subprocess
from pathlib import Path
import argparse
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap, Normalize, ListedColormap
from matplotlib.ticker import FuncFormatter
import yaml
from collections import defaultdict
import numpy as np
import pickle

# ECDF consts
EPS_GRID = np.logspace(-4, 2, 300)
REF_EPS = (0.01, 0.05, 0.10, 1.0)
REF_LABELS = ('1%', '5%', '10%', '2x')
VAR_SHORT = {
    'Number Density' : 'ND',
    'Specific Activity' : 'SA',
    'Total Decay Heat' : 'DH'
}

# Heatmap consts
THRESH = 1e-3
TARGET = 100 - THRESH
GEQ = rf'$\geq {TARGET:.3f}\%$'
BAD_COLOR = 'lightgray'
INF_COLOR= 'k'
ZERO_COLOR = '#2a9d8f'

def new_store():
    return {
        'ratios': [], 'weights' : [],
        'elements': [], 'unit' : None
    }

def extract_run_lbl(adf, code):
    return [r for r in adf['run_lbl'] if code in r][0]

def extract_single_adf_val(adf, nuc):
    return adf.loc[adf['nuclide'] == nuc, 'value'].iat[0]

def to_builtin(obj):
    if isinstance(obj, dict):
        return {
            key: to_builtin(value)
            for key, value in obj.items()
        }

    if isinstance(obj, list):
        return [to_builtin(value) for value in obj]

    if isinstance(obj, tuple):
        return tuple(to_builtin(value) for value in obj)

    return obj

def process_input_file(inp):

    with open(inp, 'r') as f:
        inputs = yaml.safe_load(f.read())

    case_name = inputs.pop(
        'Case Name',
        '_'.join(inputs['Run Specs'].keys())
    ).replace(' ', '_')
    alara_cases = defaultdict(list)
    fispact_cases = defaultdict(list)
    for run_lbl, values in inputs['Run Specs'].items():
        if 'ALARA' in values[0]:
            if len(values) == 2:
                alara_cases[run_lbl].extend(values)
            else:
                raise ValueError(
                    'Invalid input. ALARA data requires two nested values: ' \
                    r'"ALARA" and "{alaralib_name}".'
                )
        elif 'FISPACT' in values[0]:
            fispact_cases[run_lbl].extend(values)

        else:
            raise ValueError('Only FISPACT-II and ALARA data acceptable.')

    return case_name, alara_cases, fispact_cases

def filter_elements():

    alara_elements = set()
    with open('../../../../data/nuclib.std', 'r') as f:
        for line in f:
            first_val = line.split()[0]
            if all(char.isalpha() for char in first_val):
                alara_elements.add(first_val.capitalize())

    fispact_elements = set()
    for fispact_output_file in Path('fispact_outputs').iterdir():
        with open(fispact_output_file, 'r') as f:
            for line in f:
                if 'FISPACT title:' in line:
                    fispact_elements.add(
                        line
                        .split('* EUDEMO FW')[1]
                        .split(' 2 years')[0]
                        .strip()
                    )

    return dict(sorted({
        element: elements[element] for element in 
        elements.keys() & (alara_elements & fispact_elements)
    }.items(), key=lambda item: item[1]))

### ALARA Interfacing ###
def fill_alara_template(element, lib, nGroups, truncation=1e-12):
    with open('alara_template.txt', 'r') as f:
#    with open('alara_template_shutdown.txt', 'r') as f:
        return Template(f.read()).safe_substitute(
            element=element.lower(), lib=lib,
            nGroups=nGroups, truncation=truncation
        )

def write_alara_input_file(alara_template):
    with open('alara.inp', 'w') as f:
        f.write(alara_template)

def run_alara(output_dir, element, run, timeout=None):
    output_path = f'{output_dir}/{element}_{run.replace(' ', '_')}.out'
    with open(output_path, 'w') as o:
        try:
            subprocess.run(
                        ['alara', '-v', '3', 'alara.inp'],
                        stdout=o,
                        timeout=timeout
                )
        except subprocess.TimeoutExpired as e:
            Path(output_path).unlink()
            return None, e

    return output_path, None

# ECDF
def fold_difference(raw):
    '''
    Create an array of fold differences: max(ratio, 1/ratio)
    '''

    raw = np.asarray(raw, dtype=float)
    with np.errstate(divide='ignore', invalid='ignore'):
        return np.where(raw >= 1, raw, 1/raw)

def weighted_ecdf(series, weights):
    '''
    Sort an error array and return it alongside an array of their cumulative weights
    '''

    series = np.asarray(series, dtype=float)
    weights = np.asarray(weights, dtype=float)

    if series.shape != weights.shape:
        raise ValueError('series and weights must have the same shape.')

    valid = np.isfinite(series) & np.isfinite(weights) & (weights >= 0)
    series = series[valid]
    weights = weights[valid]

    order = np.argsort(series, kind='stable')
    series = series[order]
    weights = weights[order]

    cumulative = np.cumsum(weights)

    return series, cumulative

def ecdf_curves(raw_ratios, weights, n_elements, grid=EPS_GRID):
    '''
    Create weighted and unweighted ECDF arrays, as a function of tolerances.
    '''

    epsilon = fold_difference(raw_ratios) - 1.0
    adjusted_weights = np.where(np.isfinite(weights), weights, 0.0)
    finite = np.isfinite(epsilon)
    e_sorted = np.sort(epsilon[finite], kind='stable')
    n_valid = len(e_sorted)

    e_weighted, cumulative_weighted = weighted_ecdf(epsilon, adjusted_weights)

    idx_weighted = np.searchsorted(e_weighted, grid, side='right')
    cumulative_weighted = np.concatenate(([0.0], cumulative_weighted))
    within_weighted = np.clip(cumulative_weighted[idx_weighted] / n_elements, 0.0, 1.0)

    idx_unweighted = np.searchsorted(e_sorted, grid, side='right')
    within_unweighted = np.clip(idx_unweighted / max(n_valid, 1), 0.0, 1.0)

    return within_weighted, within_unweighted

def _style_ecdf_axis(ax, ylabel):
    ax.set_xscale('log')
    ax.set_xlim(EPS_GRID[0], EPS_GRID[-1])
    ax.set_ylim(0, 1.02)
    ax.xaxis.set_major_formatter(FuncFormatter(lambda v, _: f'{100*v:g}%'))
    ax.yaxis.set_major_formatter(FuncFormatter(lambda v, _: f'{100*v:g}%'))
    for e, lbl in zip(REF_EPS, REF_LABELS):
        ax.axvline(e, color='gray', ls=':', lw=0.8)
        ax.text(
            e, 1.01, lbl,
            transform=ax.get_xaxis_transform(),
            ha='center', va='bottom', fontsize=8, color='gray'
        )
    ax.grid(alpha=0.3)
    ax.set_xlabel((
        r'Deviation between FISPACT-II and ALARA: $max(ratio, ^{1} / _{ratio}) - 1$'
    ))
    ax.set_ylabel(ylabel)

def plot_element_ecdf_all_variables(element, variables, path):
    '''
    Plot response-weighted and by-nuclide unweighted ECDFs together for all
        variables, per-element. Save weighted tolerance data to dictionaries
        to be stored in a DataFrame for all ECDFs, which can be used to produce
        cross-ECDF heat maps.
    '''

    fig, ax = plt.subplots(figsize=(10, 6.5))
    table_rows = []
    df_rows = []

    for i, (var, store) in enumerate(variables.items()):
        sel = np.asarray(store['elements']) == element
        if not sel.any():
            continue

        raw = np.asarray(store['ratios'], dtype=float)[sel]
        w = np.asarray(store['weights'], dtype=float)[sel]
        within_w, within_u = ecdf_curves(raw, w, 1)
        color = f'C{i}'

        ax.step(
            EPS_GRID, within_w, where='post', color=color, lw=2,
            label=f'{var} response (n={int(sel.sum())} nuclides)'
        )
        ax.step(
            EPS_GRID, within_u, where='post', color=color, lw=1.2,
            ls='--', label=f'{var} nuclide count'
        )

        ref_w, _ = ecdf_curves(raw, w, 1, grid=np.array(REF_EPS))
        table_rows.append((VAR_SHORT.get(var, var), ref_w))

        df_row = {'element' : element, 'var' : var}
        for tol, r in zip(EPS_GRID, within_w):
            df_row[tol] = r * 100

        df_rows.append(df_row)

    if not table_rows:
        plt.close(fig)
        return []

    header = (
        'Response within tolerance\n' + ' ' * 4
        + ''.join(f'{l:>9}' for l in REF_LABELS)
    )
    body = [
        f'{short:<4}' + ''.join(f'{100 * r:8.3f}%' for r in ref_w)
        for short, ref_w in table_rows
    ]
    ax.text(
        0.99, 0.2, header + '\n' + '\n'.join(body),
        transform=ax.transAxes, ha='right', va='top', family='monospace',
        fontsize=9,
        bbox=dict(boxstyle='round', fc='white', ec='gray', alpha=0.9)
    )

    _style_ecdf_axis(ax, 'Fraction within tolerance')
    ax.set_title(f'{element} (ND, SA, DH): FISPACT-II vs. ALARA', pad=16)
    ax.legend(
        loc='lower right', bbox_to_anchor=(0.99, 0.215),
        fontsize=8, borderaxespad=0
    )
    fig.savefig(path, bbox_inches='tight')
    plt.close(fig)

    return df_rows

def plot_ecdfs_by_element(variables, out_dir):
    out_dir.mkdir(parents=True, exist_ok=True)

    elems = {e for v in variables.values() for e in v['elements']}
    all_rows = []
    for e in sorted(elems, key=lambda x: elements[x]):
        df_rows = plot_element_ecdf_all_variables(e, variables, out_dir / f'{e}_ecdf.png')
        all_rows.extend(df_rows)

    return all_rows

# Heatmap functions
def create_cmap(cmap_name):
    base = plt.get_cmap(cmap_name)(np.linspace(0, 1, 256))
    cmap = LinearSegmentedColormap.from_list(f'{cmap_name}_smooth', base, N=4096)
    return cmap, Normalize(vmin=0, vmax=100)

def order_elements(df):
    return sorted(df['element'].unique(), key= lambda x: elements[x])

def order_tols(df):
    return sorted(
        (col for col in df.columns if col not in ('element', 'var')),
        key=float
    )[::-1]

def get_vals(df, var, order, tols):
    sub = (
        df[df['var'] == var]
        .set_index('element')
        .reindex(order)[tols]
    )
    return sub.to_numpy(dtype=float).T

def add_side_key(ax, im, cbar_label):
    cax = ax.inset_axes([1.01, 0.55, 0.18, 0.05])
    cbar = ax.get_figure().colorbar(im, cax=cax, orientation='horizontal')
    cbar.solids.set_edgecolor('face')
    cbar.set_label(cbar_label, fontsize='large')
    cax.xaxis.set_label_position('top')
    cax.xaxis.set_ticks_position('bottom')
    ax.legend(
        loc='upper left', bbox_to_anchor=(1.01, 0.45),
        fontsize='large', borderaxespad=0
    )
    return cbar

def draw_panel(ax, tols, vals, order, cmap, norm):
    '''
    Draw a heatmap wherein each "column" corresponds to a single element's linearized
        view of an ECDF. Allows for comparing single variable ECDF's together across
        elements.
    '''
    n_elements = len(order)
    tol_pct = np.array(tols, float)[::-1] * 100
    lt = np.log10(tol_pct)
    mid = (lt[1:] + lt[:-1]) / 2
    edges = 10 ** np.concatenate(
        [[2 * lt[0] - mid[0]], mid, [2 * lt[-1] - mid[-1]]]
    )

    data = vals[::-1]
    im = ax.pcolormesh(
        np.arange(n_elements + 1) - 0.5, edges, data, cmap=cmap, norm=norm
    )
    ax.set_yscale('log')

    # Smallest tolerance at which each element reaches the target coverage
    reached = data >= TARGET
    has_cross = reached.any(axis=0)
    indices = np.arange(n_elements)
    cols = indices[has_cross]
    miss = indices[~has_cross]
    ys = edges[reached.argmax(axis=0)]

    ax.hlines(
        ys[has_cross],
        cols - 0.5,
        cols + 0.5,
        colors='k',
        linewidth=4,
        label='Smallest tolerance where response within ' + GEQ
    )
    ax.scatter(
        miss, np.ones(len(miss)), transform=ax.get_xaxis_transform(),
        marker='^', s=60, color='k', clip_on=False,
        label=rf'Never {GEQ} within {tol_pct.max():.0e}%'
    )

    ax.set_xticks(indices)
    ax.set_xticklabels(order, rotation=45, ha='right', rotation_mode='anchor')
    ax.set_xticks(np.arange(n_elements + 1) - 0.5, minor=True)
    ax.grid(which='minor', axis='x', color='k', alpha=0.25, linewidth=0.5)
    ax.grid(axis='y', alpha=0.2, color='k', linewidth=0.5)
    ax.tick_params(axis='x', which='minor', length=0)

    ax.set_ylabel('Tolerance (%)', size='large')

    return im

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--choose_elements', '-c', required=False, nargs='*')
    parser.add_argument('--existing_alara_outputs', '-e', action='store_true')
    parser.add_argument('--trUncation', '-u', default=1e-12, type=float)
    parser.add_argument('--time', '-t', default=0.0, type=float)
    parser.add_argument('--heatmap', '-m', action='store_true')
    args = parser.parse_args()


    # Load in run data
    case_name, alara_cases, fispact_cases = process_input_file('workflow_input.yaml')
    case_dir = Path(f'{case_name}_{args.time}s')
    case_dir.mkdir(parents=True, exist_ok=True)

    # Global variables
    variables = {
        v: new_store() for v in ['Number Density', 'Specific Activity', 'Total Decay Heat']
    }

    if not args.choose_elements:
        iter_elements = filter_elements()
    else:
        iter_elements = args.choose_elements

    for element in iter_elements:
        if element == 'Pa':
            continue

        output_dir = case_dir / element
        output_dir.mkdir(parents=True, exist_ok=True)

        # Run ALARA single element irradiation
        for alara_run, params in alara_cases.items():
            if not args.existing_alara_outputs:
                lib = params[1]
                try:
                    nGroups = int(lib.split('_')[2])
                except IndexError: # EAF-2010 case
                    nGroups = 175
                truncation = args.trUncation
                run_success = False
                while not run_success:
                    alara_template = fill_alara_template(
                        element, lib, nGroups, truncation=truncation
                    )
                    write_alara_input_file(alara_template)
                    alara_output_path, err = run_alara(
                        output_dir, element, alara_run, timeout=60
                    )

                    if not isinstance(err, subprocess.TimeoutExpired):
                        run_success = True
                        break

                    truncation *= 10
            else:
                alara_output_path = list(
                    output_dir.glob(
                        f'{element.capitalize()}_{alara_run.replace(' ', '_')}.out')
                )[0]

            alara_cases[alara_run].append(alara_output_path)

        # Collect and load data to an ADF
        for run in fispact_cases:
            fispact_cases[run].append(
                f'fispact_outputs/{element.capitalize()}_T17.fis'
                #f'fispact_outputs/{element.capitalize()}_eaf2010.fis'
            )

        data_source = dict()
        for d in [alara_cases, fispact_cases]:
            for run, params in d.items():
                data_source[run] = params[-1]

        aop_lib = aop.DataLibrary()
        adf = aop.DataLibrary.make_entries(aop_lib, data_source)

        # Iterate by variable
        for var in variables.keys():
            store = variables[var]
            
            single_var = adf.filter_rows({
                'variable' : adf.VARIABLE_ENUM[var], 'time' : args.time
            })
            alara = single_var.filter_rows({
                'run_lbl' : extract_run_lbl(adf, 'ALARA')
            })
            fispact = single_var.filter_rows({
                'run_lbl' : extract_run_lbl(adf, 'FISPACT') 
            })

            fis_vals = dict(zip(fispact['nuclide'], fispact['value']))
            alara_vals = dict(zip(alara['nuclide'], alara['value']))
            fis_total = fis_vals.pop('total')
            alara_total = alara_vals.pop('total')

            # Calculate ratios, weights and store
            vals, ratios, weights = [], [], []
            with np.errstate(divide='ignore', invalid='ignore'):
                ft, at = np.float64(fis_total), np.float64(alara_total)
                for nuc in set(fis_vals) & set(alara_vals):
                    fv, av = np.float64(fis_vals[nuc]), np.float64(alara_vals[nuc])
                    ratio = fv / av
                    vals.append(max(fv, av))
                    ratios.append(ratio)
                    weights.append(0.5 * (fv/ft + av/at))

            store['ratios'].extend(ratios)
            store['elements'].extend([element] * len(vals))
            store['weights'].extend(weights)


    # Save variables defaultdict to a pkl file to be read in individually,
    # can skip previous data production
    with open(case_dir / 'pickle.pkl', 'wb') as f:
        pickle.dump(to_builtin(variables), f)


    # ECDF
    ecdf_rows = plot_ecdfs_by_element(variables, case_dir / 'ecdf_by_element')
    df = pd.DataFrame(ecdf_rows)
    df.to_csv(
        case_dir / f'ecdf_tolerances.csv',
        index=False
    )

    # Optional heatmap plotting
    if args.heatmap:
        tols = order_tols(df)
        order = order_elements(df)
        cmap, norm = create_cmap('plasma')
        for var in variables:
            heatmap_fig, heatmap_ax = plt.subplots(figsize=(28,9))
            vals = get_vals(df, var, order, tols)
            im = draw_panel(heatmap_ax, tols, vals, order, cmap, norm)
            heatmap_ax.set_ylabel('Tolerance (%)', size='large')
            heatmap_ax.set_xlabel('Elements', size='large')
            add_side_key(heatmap_ax, im, 'Response Within Tolerance (%)')

            heatmap_fig.savefig(
                case_dir / f'{var.replace(' ', '_')}.png',
                bbox_inches='tight',
                dpi=200
            )
            plt.close(heatmap_fig)

if __name__ == '__main__':
    main()
