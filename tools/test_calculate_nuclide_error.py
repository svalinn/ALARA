
import numpy as np
import pytest

from calculate_nuclide_error import (
    prep_arrays_for_measures,
    make_child_sum_ratio,
    make_norm_abs_diff,
    make_cosine_similarity,
    make_norm_rel_diff,
    create_error_measures
)


@pytest.fixture
def input_data():
    parent_nuc = "he:4"
    child_nucs = ["he:3", "he:4", "li:5"]

    predicted_sample_response_array = np.array([
        [2.0, 100.0, 0.0],
        [0.0, 150.0, 3.0],
        [4.0, 200.0, 4.0]
    ])

    true_sample_response_array = np.array([
        [1.0, 200.0, 3.0],
        [2.0, 150.0, 0.0],
        [0.0, 200.0, 0.0]
    ])

    return parent_nuc, child_nucs, predicted_sample_response_array, true_sample_response_array


@pytest.fixture
def data_prepared_for_measures(input_data):
    parent_nuc, child_nucs, predicted_sample_response_array, true_sample_response_array = input_data

    response_sum_p_removed, rel_sample_responses_p_removed = (
        prep_arrays_for_measures(
            parent_nuc, child_nucs, predicted_sample_response_array, true_sample_response_array
        )
    )

    return response_sum_p_removed, rel_sample_responses_p_removed


def test_prep_arrays_for_measures(input_data):
    parent_nuc, child_nucs, predicted_sample_response_array, true_sample_response_array = input_data

    response_sum, rel_responses = prep_arrays_for_measures(
        parent_nuc, child_nucs, predicted_sample_response_array, true_sample_response_array
    )

    np.testing.assert_allclose(
        response_sum['predicted'], [2.0, 3.0, 8.0]
    )

    np.testing.assert_allclose(
        response_sum['true'], [4.0, 2.0, 0.0]
    )

    expected_predicted_sample_response_array = np.array([
        [1.0, 0.0],
        [0.0, 1.0],
        [0.5, 0.5]
    ])

    expected_true_sample_response_array = np.array([
        [0.25, 0.75],
        [1.0, 0.0],
        [np.nan, np.nan]
    ])

    np.testing.assert_allclose(
        rel_responses['predicted'],
        expected_predicted_sample_response_array,
        equal_nan=True
    )

    np.testing.assert_allclose(
        rel_responses['true'],
        expected_true_sample_response_array,
        equal_nan=True
    )


def test_make_child_sum_ratio(data_prepared_for_measures):
    response_sum, _ = data_prepared_for_measures

    child_sum_ratio = make_child_sum_ratio(
        response_sum['predicted'],
        response_sum['true']
    )

    np.testing.assert_allclose(
        child_sum_ratio,
        [0.5, 1.5, np.nan],
        equal_nan=True
    )


def test_make_norm_abs_diff(data_prepared_for_measures):
    _, rel_responses = data_prepared_for_measures

    diff_test_ref = (
        rel_responses['predicted'] - rel_responses['true']
    )

    norm_abs_diff = make_norm_abs_diff(diff_test_ref)

    np.testing.assert_allclose(
        norm_abs_diff,
        [np.sqrt(1.125), np.sqrt(2.0), np.nan],
        equal_nan=True
    )


def test_make_cosine_similarity(data_prepared_for_measures):
    _, rel_responses = data_prepared_for_measures

    cosine_similarity = make_cosine_similarity(
        rel_responses['predicted'],
        rel_responses['true']
    )

    np.testing.assert_allclose(
        cosine_similarity,
        [0.316227766, 0.0, np.nan],
        equal_nan=True
    )


def test_make_norm_rel_diff(data_prepared_for_measures):
    _, rel_responses = data_prepared_for_measures

    diff_test_ref = (
        rel_responses['predicted'] - rel_responses['true']
    )

    norm_rel_diff = make_norm_rel_diff(
        diff_test_ref,
        rel_responses['true']
    )

    np.testing.assert_allclose(
        norm_rel_diff,
        [np.sqrt(10.0), 1.0, 0.0],
        equal_nan=True
    )


def test_create_error_measures(data_prepared_for_measures):
    response_sum, rel_responses = data_prepared_for_measures

    err_measures = create_error_measures(
        response_sum,
        rel_responses
    )

    np.testing.assert_allclose(
        err_measures['child_sum_ratio'],
        [0.5, 1.5, np.nan],
        equal_nan=True
    )

    np.testing.assert_allclose(
        err_measures['norm_abs_diff'],
        [np.sqrt(1.125), np.sqrt(2.0), np.nan],
        equal_nan=True
    )

    np.testing.assert_allclose(
        err_measures['cosine_similarity'],
        [0.316227766, 0.0, np.nan],
        equal_nan=True
    )

    np.testing.assert_allclose(
        err_measures['norm_rel_diff'],
        [np.sqrt(10.0), 1.0, 0.0],
        equal_nan=True
    )
