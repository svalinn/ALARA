#This script produces an ALARA material library using the standard element library available in the ALARA/data folder.

elelib_lines = open('../data/elelib.std', 'r').readlines()

with open('../data/elelib_to_matlib.txt', 'w') as etm:
    for elelib_line in elelib_lines:
        if len(elelib_line.split()) == 5: # the 1st line corresponding to each element contains the elemental symbol, molar mass, atomic number, density, and # of isotopes
            etm.write('mat:'+elelib_line.split()[0]+ '\t' + elelib_line.split()[3] + '\t' + '1' + '\n' + elelib_line.split()[0]
                      + '\t' + '1.000000E+02' + '\t' + elelib_line.split()[2] + '\n\n')