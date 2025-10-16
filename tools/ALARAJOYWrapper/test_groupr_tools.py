# Import packages
import pytest
import groupr_tools
import reaction_data as rxd

mt_dict = rxd.process_mt_data(rxd.load_mt_table('mt_table.csv'))

@pytest.mark.parametrize(
    "material_id, MTs, element, A, mt_dict, exp",
    [
        (
            2631,
            [1, 2, 3, 4, 5, 11, 16, 17, 22, 24, 28, 29, 32, 33, 34, 41, 42,
             44, 45, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
             65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
             91, 102, 103, 104, 105, 106, 107, 108, 111, 112, 115, 116, 117],
            'Fe',
            56,
            mt_dict,
            '''groupr/
 20 21 0 31/
 2631 17 0 11 0 1 1 1/
 "26-Fe-56 for TENDL 2017"/
 293.16
 0/
 3 1 "(n,total)" /
 3 2 "(z,z0)" /
 3 3 "(z,nonelas.)" /
 3 4 "(z,n)" /
 3 5 "(z,anything)" /
 3 11 "(z,2nd)" /
 3 16 "(z,2n)" /
 3 17 "(z,3n)" /
 3 22 "(z,nα)" /
 3 24 "(z,2nα)" /
 3 28 "(z,np)" /
 3 29 "(z,n2α)" /
 3 32 "(z,nd)" /
 3 33 "(z,nt)" /
 3 34 "(z,n3He)" /
 3 41 "(z,2np)" /
 3 42 "(z,3np)" /
 3 44 "(z,n2p)" /
 3 45 "(z,npα)" /
 3 51 "(z,n1)" /
 3 52 "(z,n2)" /
 3 53 "(z,n3)" /
 3 54 "(z,n4)" /
 3 55 "(z,n5)" /
 3 56 "(z,n6)" /
 3 57 "(z,n7)" /
 3 58 "(z,n8)" /
 3 59 "(z,n9)" /
 3 60 "(z,n10)" /
 3 61 "(z,n11)" /
 3 62 "(z,n12)" /
 3 63 "(z,n13)" /
 3 64 "(z,n14)" /
 3 65 "(z,n15)" /
 3 66 "(z,n16)" /
 3 67 "(z,n17)" /
 3 68 "(z,n18)" /
 3 69 "(z,n19)" /
 3 70 "(z,n20)" /
 3 71 "(z,n21)" /
 3 72 "(z,n22)" /
 3 73 "(z,n23)" /
 3 74 "(z,n24)" /
 3 75 "(z,n25)" /
 3 76 "(z,n26)" /
 3 77 "(z,n27)" /
 3 78 "(z,n28)" /
 3 79 "(z,n29)" /
 3 80 "(z,n30)" /
 3 91 "(z,nc)" /
 3 102 "(z,γ)" /
 3 103 "(z,p)" /
 3 104 "(z,d)" /
 3 105 "(z,t)" /
 3 106 "(z,3He)" /
 3 107 "(z,α)" /
 3 108 "(z,2α)" /
 3 111 "(z,2p)" /
 3 112 "(z,pα)" /
 3 115 "(z,pd)" /
 3 116 "(z,pt)" /
 3 117 "(z,dα)" /
 0
 0/
stop
''' )]
)
def test_fill_input_template(material_id, MTs, element, A, mt_dict, exp):
    obs = groupr_tools.fill_input_template(material_id, MTs, element, A, mt_dict)
    assert obs == exp