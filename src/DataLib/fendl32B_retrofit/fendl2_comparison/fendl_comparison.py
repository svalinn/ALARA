# Import packages
import argparse
import matplotlib.pyplot as plt
import numpy as np
from pandas import read_csv
from ast import literal_eval
from pathlib import Path
import sys
sys.path.append('../groupr_tools.py')
from groupr_tools import elements

vitamin_J = np.array([
   0.00001000, 0.10000000, 0.41399000, 0.53158000, 0.68256000, 0.87642000,
   1.12540000, 1.44500000, 1.85540000, 2.38240000, 3.05900000, 3.92790000,
   5.04350000, 6.47600000, 8.31530000, 10.6770000, 13.7100000, 17.6030000,
   22.6030000, 29.0230000, 37.2670000, 47.8510000, 61.4420000, 78.8930000,
   101.300000, 130.070000, 167.020000, 214.450000, 275.360000, 353.580000,
   454.000000, 582.950000, 748.520000, 961.120000, 1234.10000, 1584.60000,
   2034.70000, 2248.70000, 2485.20000, 2612.60000, 2746.50000, 3035.40000,
   3354.60000, 3707.40000, 4307.40000, 5530.80000, 7101.70000, 9118.80000,
   10595.0000, 11709.0000, 15034.0000, 19305.0000, 21875.0000, 23579.0000,
   24176.0000, 24788.0000, 26058.0000, 27000.0000, 28500.0000, 31828.0000,
   34307.0000, 40868.0000, 46309.0000, 52475.0000, 56562.0000, 67379.0000,
   72000.0000, 79500.0000, 82500.0000, 86517.0000, 98037.0000, 111090.000,
   116790.000, 122770.000, 129070.000, 135690.000, 142640.000, 149960.000,
   157640.000, 165730.000, 174220.000, 183160.000, 192550.000, 202420.000,
   212800.000, 223710.000, 235180.000, 247240.000, 273240.000, 287250.000,
   294520.000, 297200.000, 298500.000, 301970.000, 333730.000, 368830.000,
   387740.000, 407620.000, 450490.000, 497870.000, 523400.000, 550230.000,
   578440.000, 608100.000, 639280.000, 672060.000, 706510.000, 742740.000,
   780820.000, 820850.000, 862940.000, 907180.000, 961640.000, 1002600.00,
   1108000.00, 1164800.00, 1224600.00, 1287300.00, 1353400.00, 1422700.00,
   1495700.00, 1572400.00, 1653000.00, 1737700.00, 1826800.00, 1920500.00,
   2019000.00, 2122500.00, 2231300.00, 2306900.00, 2345700.00, 2365300.00,
   2385200.00, 2466000.00, 2592400.00, 2725300.00, 2865000.00, 3011900.00,
   3166400.00, 3328700.00, 3678800.00, 4065700.00, 4493300.00, 4723700.00,
   4965900.00, 5220500.00, 5488100.00, 5769500.00, 6065300.00, 6376300.00,
   6592400.00, 6703200.00, 7046900.00, 7408200.00, 7788000.00, 8187300.00,
   8607100.00, 9048400.00, 9512300.00, 10000000.0, 10513000.0, 11052000.0,
   11618000.0, 12214000.0, 12523000.0, 12840000.0, 13499000.0, 13840000.0,
   14191000.0, 14550000.0, 14918000.0, 15683000.0, 16487000.0, 16905000.0,
   17333000.0, 19640000.0
][::-1]) / 1e6

def args():
   """"
   Configure argparser for the FENDL3.2b/FENDL2.0 comparison script.

   Arguments:
      None

   Returns:
      argparse.Namespace: Argparse object that contains the user specified
         arguments for executing the script.
   """

   parser = argparse.ArgumentParser()

   parser.add_argument(
      '--element', '-e', required=True, nargs='?',
      help='''Chemical symbol for the selected element to analyze.'''
   )
   parser.add_argument(
      '--mass_number', '-a', required=True, nargs='?',
      help = '''Mass number for the selected isotope. If the target is an
      isomer, type "m" after the mass number in the same string.'''
   )
   parser.add_argument(
      '--emitted_particles', '-p', required=True, nargs='?',
      help='''String of the emission from the target reaction.
      For example, for an (n,γ) reaction, type "gamma".'''
   )
   
   return parser.parse_args()

def prepend_with_zeros(cross_sections):
   """
   Include zeroes into the beginning of a list such that the total length of
      of the list is 175, corresponding to the Vitamin-J group structure.
   
   Arguments:
      cross_sections (list): List of cross sections, of length less than or
         equal to 175.
   
   Returns:
      cross_sections (list): Potentially modified list of cross sections, with
         zeroes prepended to bring the length of the list to 175.
   """
   
   current_length = len(cross_sections)
   zeros_needed = 175 - current_length
   cross_sections = [0] * zeros_needed + cross_sections

   return cross_sections


def fendl3_cross_sections(gendf_data_csv, element, A, emitted_particles):
   """
   Extracts the cross section data for a specific isotope and activation
      reaction for FENDL3.2b activation data that was converted to a
      Vitamin-J groupwise structure with a Vitamin-E weight function using the
      NJOY GROUPR module.

   Arguments:
      gendf_data_csv (str): Path to a CSV containing the extracted cross
         section data from a GENDF file produced by processing a TENDL 2017
         file with GROUPR.
      element (str): Chemical symbol for the selected element.
      A (str or int): Mass number for the selected isotope/isomer.
      emitted_particles (str): Particle product(s) of a neutron activation,
         written together in a single string.
   
   Returns:
      cross_sections (numpy.array): Array containing cross section values for
         the 175 Vitamin-J energy groups.
   """

   fendl3_df = read_csv(gendf_data_csv)

   Z = elements[element]
   M = 1 if 'm' in str(A).lower() else 0
   A = int(str(A).lower().split(' ')[0].split('m')[0])
   pkZA = (Z * 1000 + A) * 10 + M

   single_isotope_data = fendl3_df[fendl3_df['Parent KZA'] == pkZA]
   single_isotope_reaction = single_isotope_data[
      single_isotope_data['Emitted Particles'] == emitted_particles
   ]
   cross_sections = prepend_with_zeros(list(
      single_isotope_reaction['Cross Sections'].apply(literal_eval)
   )[0])
   
   return np.array(cross_sections)

def fendl2_cross_sections(element, A, emitted_particles, dir = '.'):
   """
   Extracts the cross section data for a specific isotope and activation
      reaction for groupwise FENDL2.0 activation data in a GENDF file.

   Arguments:
      element (str): Chemical symbol for the selected element.
      A (str or int): Mass number for the selected isotope/isomer.
      dir (str, optional): Path to the directory containing the GENDF file.
         Defaults to the present working directory ('.').
   
   Returns:
      cross_sections (numpy.array): Array containing cross section values for
         the 175 Vitamin-J energy groups.
   """
    
   dir = Path(dir)
   gendf_data = list(dir.glob(f'*{element}*{A}*{emitted_particles}.gendf'))[0]
  
   with open(gendf_data, 'r') as f:
      gendf = f.read()
   gendf_lines = gendf.split('\n')[3:]

   cross_sections = []
   for line in gendf_lines:
      values = line.split(' ')
      for value in values:
         if value:
            cross_sections.append(float(value))
    
   return np.array(prepend_with_zeros(cross_sections))

def replace_with_greek(emitted_particles):
   """
   Replaces Anglicized Greek letters to the Greek alphabet.

   Arguments:
      emitted_particles (str): String of particle names which may or may not
         contain Greek letters written out in English.

   Returns:
      emitted_particles (str): String of particle names wherein any particles
         that contain a Greek letter are written in Greek.
   """

   greek_dict = {'gamma': 'γ', 'alpha': 'α'}
   
   for eng, grk in greek_dict.items():
      emitted_particles = emitted_particles.replace(eng, grk)
   
   return emitted_particles

def calculate_stats(array1, array2):
   """
   Calculate the mean and maximum percent differences of two numpy arrays.

   Arguments:
      array1 (numpy.array): A numpy array containing the cross section data
         from one FENDL version.
      array2 (numpy.array): A numpy array containing the cross section data
         from a different FENDL version.

   Returns:
      mean_percent_diff (float): The calculated mean percent difference
         between the two arrays.
      max_percent_diff (float): The calculated maximum percent difference
         between the two arrays.
   """

   percent_diffs = np.array([
      (array1[i] - array2[i]) / array1[i] * 100 if array1[i] != 0 else 0.0
      for i in range(len(array1))
   ])

   mean_percent_diff = np.mean(np.abs(percent_diffs))
   max_percent_diff = np.max(np.abs(percent_diffs))

   return mean_percent_diff, max_percent_diff

def comp_plot(f2, f3, emitted_particles, mean_percent_diff, max_percent_diff):
   """
   Create a comparative plot for the cross section data from FENDL3.2b and
      FENDL2.0 for a specific isotope.

   Arguments:
      f2 (numpy.array): A numpy array containing the cross section data
         from FENDL2.0.
      f3 (numpy.array): A numpy array containing the cross section data
         from FENDL3.2b.
      emitted_particles (str): Particle product(s) of a neutron activation,
         written together in a single string.
      mean_percent_diff (float): The calculated mean percent difference
         between the two arrays.
      max_percent_diff (float): The calculated maximum percent difference
         between the two arrays.

   Returns:
      None           
   """

   groups = range(175)
   plt.figure(figsize=(15,8))
   plt.plot(groups, f3, label='GROUPR-processed FENDL3.2b',
            alpha=0.5, color='b')
   plt.plot(groups, f2, label='FENDL2.0', alpha=0.5, color = 'r')
   plt.xlabel('Energy (MeV)', fontsize='medium')
   plt.ylabel('Cross Section (b)', fontsize='medium')
   plt.title(
      ('Comparison of FENDL3.2b GROUPR-processed cross sections with ' 
      f'FENDL2.0 data \n for the {args().element}-{args().mass_number} '
      f'(n,{emitted_particles}) reaction with the 175 group Vitamin-J '
      'structure \n and the Vitamin-E micro-flux weighting function'),
      fontsize='x-large'
      )
   plt.yscale('log')
   xtick_indices = [0, 20, 40, 60, 80, 100, 120, 140, 160, 174]
   xtick_labels = [f'{vitamin_J[i]:.2e}' for i in xtick_indices]
   plt.xticks(xtick_indices, xtick_labels, rotation=45)
   plt.gca().invert_xaxis()
   plt.legend(
      title=(
         f'Mean percent difference: {mean_percent_diff:.2f}% \n'
         f'Max percent difference: {max_percent_diff:.2f}%'
         ), 
      fontsize='medium', 
      title_fontsize='medium', 
   )
   plt.grid()
   plt.savefig(
      (
         f'fendl_comp_for_{args().element}-{args().mass_number}_'
         f'(n,{emitted_particles}).png')
   )

##############################################################################

def fendl_comparison():
   """
   Main method when run as a command line script.
   """

   emitted_particles = replace_with_greek(args().emitted_particles)
   f2 = fendl2_cross_sections(args().element, args().mass_number,
                              emitted_particles)
   f3 = fendl3_cross_sections('../cumulative_gendf_data.csv',
                              args().element, args().mass_number,
                              emitted_particles)

   mean_percent_diff, max_percent_diff = calculate_stats(f2,f3)
   comp_plot(f2, f3, emitted_particles, mean_percent_diff, max_percent_diff)

##############################################################################

if __name__ == '__main__':
   fendl_comparison()