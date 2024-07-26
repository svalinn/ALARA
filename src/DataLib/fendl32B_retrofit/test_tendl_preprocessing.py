# Import packages
import pytest
import tendl_preprocessing as tpp

@pytest.mark.parametrize(
    "endf_file, exp",
    [
        ('endf_test1.tendl',
         [2631,
          [1, 2, 3, 4, 5, 11, 16, 17, 22, 24, 28, 29, 32, 33, 34, 41, 42, 44,
           45, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
           67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 91, 102,
           103, 104, 105, 106, 107, 108, 111, 112, 115, 116, 117]]),

        ('endf_test2.tendl',
         [125,
          [1, 2, 102]]),
        
        ('endf_test3.tendl',
         [325,
          [1, 2, 4, 24, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
           64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
           81, 102, 103, 105]]),

        ('endf_test4.tendl',
         [9228,
          [1, 2, 4, 5, 16, 17, 18, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
           62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
           79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 91, 102, 649, 800, 801,
           802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814,
           815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827,
           828, 829, 830, 831, 832, 833, 834, 835]])
    ]
)
def test_extract_endf_specs(endf_file, exp):
    dir = './files_for_tests'
    obs = tpp.extract_endf_specs(f'{dir}/{endf_file}')
    assert obs == exp