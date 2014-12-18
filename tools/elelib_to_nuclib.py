#!/bin/env python

# This script reads an ALARA elelib file and prints an new elelib file with all
# the original entry plus one entry for each isotope of each element. Then end
# result is a single file (a 'nuclib') with entries for all elements and
# nuclides. The atomic masses of the nuclides are acquired PyNE. Densities of
# nuclides are adjusted from elemental densities.

import argparse

from pyne import data

def elelib_to_nuclib(filename, outfile):
    output=""
    with open(filename, 'r') as f:
        line = f.readline()
        while line !='':
            if len(line.split()) == 0:
                line = f.readline()
                continue

            elem_output = ''
            iso_output = ''
            
            elem_output += line
            l = line.split()
            ll = l[0] # atomic symbol
            elem_mass = float(l[1]) # average atomic weight, natural element
            znum = int(l[2])
            rho = float(l[3]) # mass density
            num_isos = int(l[4])
            for _ in range(num_isos):
                isoline = f.readline()
                elem_output += isoline
                anum = int(isoline.split()[0])
                atomic_mass = data.atomic_mass("{0}{1}".format(ll, anum))
                iso_output += "{0}:{1} {2:13.5E} {3:3d} {4:13.5E} 1\n    1 100\n".format(
                           ll, anum, atomic_mass, znum, rho*atomic_mass/elem_mass)
            line = f.readline()
            output += elem_output + iso_output

    with open(outfile, 'w') as f:
        f.write(output)
            
def main():

    parser = argparse.ArgumentParser(description=
             'Converts an ALARA element library to an isotope library')
    parser.add_argument('filename', help='Name of the ALARA elelib file.')
    parser.add_argument('-o', dest='output', default="nuclib",
                        help='Name the nuclib output file')

    args = parser.parse_args()
    elelib_to_nuclib(args.filename, args.output)

if __name__ == '__main__':
    main()
