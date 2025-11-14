# ALARA Output Processing

Contained within `ALARA/tools` is the Python package, `alara_output_processing`, a module desgined for the parsing of ALARA output files into Pandas DataFrame-inherited structures, with a robust set of operations to organize and process data to user specifications.

## Dependencies
- Standard Python libraries
    * [ArgParse](https://docs.python.org/3/library/argparse.html)
    * [IO](https://docs.python.org/3/library/io.html)
    * [Re](https://docs.python.org/3/library/re.html)
- Generic Python packages
    * [Pandas](https://pandas.pydata.org/docs/getting_started/install.html)


## Installation
From the directory `ALARA/tools/` run the following command to install `alara_output_processing`:
```
pip install .
```

## Usage
`alara_output_processing` can be used either as a pure Python library, or as a command line tool. From the command line, calling `alara_output_processing` with the filepath to an ALARA output file will automatically identify and parse all text-formatted tables contained in the output file and write them out to individual CSV files (corresponding to each zone/interval of the ALARA run). To run `alara_output_processing` from the command line, run:
```
alara_output_processing -f /path/to/alara/output.txt
```

Alternatively, when used as a Python library, `alara_output_processing` can read in and parse ALARA output tables as such:
```
from alara_output_processing import FileParser, ALARADFrame, DataLibrary

alara_data = FileParser('/path/to/alara/output.txt')
output_tables = alara_data.extract_tables()
```

If comparing multiple ALARA runs, a dictionary can be constructed of the form:
```
runs = {
    'run1' : '/path/to/run1/output.txt',
    'run2' : '/path/to/run2/output.txt',
}
```

This dictionary can be input directly into the function `DataLibrary.make_entries()` to create a list of dictionaries of all tabular data with the following command:
```
dfs = DataLibrary.make_entries(runs)
```
Wherein `dfs` contains entries for each table of the form:
```
df_dict = {
    'Run Label'   : (Distinguisher between runs),
    'Variable'    : (Any ALARA output variable, dependent on
                    ALARA run parameters),
    'Unit'        : (Respective unit matching the above
                    variable),
    'Data'        : (ALARADFrame containing ALARA output data
                    for the given run parameter, variable, and
                    unit)
}
```

Once processed, the following operations are available on any given `ALARADFrame`:

* `ALARADFrame.process_time_vals()`
    - Convert the cooling times of the ALARA analysis post-shutdown to floating point numbers, in either seconds or years.
* `ALARADFrame.extract_totals()`
    - Select the values from the "total" row of an ALARA output table ALARADFrame and write them out to a list.
* `ALARADFrame.filter_elements()`
    - Create a new ALARADFrame containing only the data for nuclides of a selected element or elements.
* `ALARADFrame.aggregate_small_percentages()`
    - Consolidate all rows in an ALARADFrame that do not have any cells with a contribution of more than a threshold value to the total for its respective column. Rows that do not have any cells that surpass its column's threshold are aggregated into a new "Other" row. If a row has at least one column value above the threshold, then the whole row is preserved.