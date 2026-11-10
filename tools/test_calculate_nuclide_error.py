
import numpy as np
import pytest

from calculate_nuclide_error import prep_arrays_for_measures, create_error_measures


@pytest.fixture
def input_data():
    parent_nuc = "he:4"
    child_nucs = ["he:3", "he:4", "li:5"]

    arr_Children_p_test = np.array([
        [2.0, 100.0, 0.0],
        [0.0, 150.0, 3.0],
        [4.0, 200.0, 4.0]
    ])

    arr_Children_p_ref = np.array([
        [1.0, 200.0, 3.0],
        [2.0, 150.0, 0.0],
        [0.0, 200.0, 0.0]
    ])

    return parent_nuc, child_nucs, arr_Children_p_test, arr_Children_p_ref


def test_prep_arrays_for_measures(input_data):
    parent_nuc, child_nucs, arr_test, arr_ref = input_data

    L_ref, L_test, arr_children_star_p_ref, arr_children_star_p_test = prep_arrays_for_measures(
        parent_nuc, child_nucs, arr_test, arr_ref
    )

    np.testing.assert_allclose(L_ref, [4.0, 2.0, 0.0])
    np.testing.assert_allclose(L_test, [2.0, 3.0, 8.0])

    expected_arr_star_test = np.array([
        [1.0, 0.0],
        [0.0, 1.0],
        [0.5, 0.5]
    ])

    expected_arr_star_ref = np.array([
        [0.25, 0.75],
        [1.0, 0.0],
        [np.nan, np.nan]
    ])

    np.testing.assert_allclose(arr_children_star_p_test, expected_arr_star_test, equal_nan=True)
    np.testing.assert_allclose(arr_children_star_p_ref, expected_arr_star_ref, equal_nan=True)


def test_create_error_measures(input_data):
    parent_nuc, child_nucs, arr_test, arr_ref = input_data

    L_ref, L_test, arr_children_star_p_ref, arr_children_star_p_test = prep_arrays_for_measures(
        parent_nuc, child_nucs, arr_test, arr_ref
    )

    measure_A, measure_B, measure_C = create_error_measures(
        L_ref, L_test, arr_children_star_p_ref, arr_children_star_p_test
    )

    np.testing.assert_allclose(
        measure_A, [0.5, 1.5, np.nan], equal_nan=True
    )

    np.testing.assert_allclose(
        measure_B,
        [np.sqrt(1.125), np.sqrt(2.0), np.nan],
        equal_nan=True
    )

    np.testing.assert_allclose(
        measure_C,
        [0.316227766, 0.0, np.nan],
        equal_nan=True
    )
