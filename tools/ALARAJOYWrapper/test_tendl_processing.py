# Import packages
import pytest
import tendl_processing as tp

dir = './files_for_tests'

@pytest.mark.parametrize(
    "endf_file, exp",
    [
        ('endf_test1.tendl',
         (2631,
          [1, 2, 3, 4, 5, 11, 16, 17, 22, 24, 28, 29, 32, 33, 34, 41, 42, 44,
           45, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
           67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 91, 102,
           103, 104, 105, 106, 107, 108, 111, 112, 115, 116, 117]))
    ]
)
def test_extract_endf_specs(endf_file, exp):
    obs = tp.extract_endf_specs(f'{dir}/{endf_file}')
    assert obs == exp

def test_extract_endf_specs_empty_endf():
    with pytest.raises(Exception, match='std::exception'):
        tp.extract_endf_specs(f'{dir}/endf_test2.tendl')

@pytest.mark.parametrize(
    "gendf_file, exp",
    [
        ('gendf_test1.gendf', 260560),
        ('gendf_test2.gendf', 471161),
        ('gendf_test3.gendf', 471162)
    ]
)
def test_extract_gendf_pkza(gendf_file, exp):
    obs = tp.extract_gendf_pkza(f'{dir}/{gendf_file}')
    assert obs == exp