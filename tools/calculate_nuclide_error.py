import numpy as np


def prep_arrays_for_measures(parent_nuc, child_nucs,
                             predicted_sample_response_array,
                             true_sample_response_array):
    '''
    Takes input arrays of activation responses and removes entries containing
    the original parent nuclide. Also calculates a sum of the responses across
    each row, and normalizes the array by the vector of sums.

    :param parent_nuc (str): name of irradiated nuclide 
    :param child_nucs (iterable of str): names of all possible daughter nuclides resulting from
    the irradiation of any parent nuclide (must contain the parent nuclide)
    :param predicted_sample_response_array: 2D numpy array or nested list of predicted activation responses,
    where the rows correspond to the different samples or irradiation conditions, and the columns 
    are correspond to the different daughter nuclides
    :param true_sample_response_array: 2D numpy array or nested list of ground-truth activation responses,
    where the rows correspond to the different samples or irradiation conditions, and the columns 
    are correspond to the different daughter nuclides
    '''
    parent_idx = child_nucs.index(parent_nuc)
    sample_response_arrays = {
        'predicted': np.asarray(predicted_sample_response_array),
        'true': np.asarray(true_sample_response_array)
    }
    response_sum_p_removed = {}
    rel_sample_responses_p_removed = {}
    for case in sample_response_arrays.keys():
        sample_response_array_p_removed = np.delete(
            sample_response_arrays[case], parent_idx, axis=1)
        response_sum_p_removed[case] = np.sum(sample_response_array_p_removed,
                                              axis=1)
        rel_sample_responses_p_removed[case] = np.divide(
            sample_response_array_p_removed,
            response_sum_p_removed[case].reshape(-1, 1),
            out=np.full_like(sample_response_array_p_removed, np.nan),
            where=sample_response_array_p_removed.reshape(-1, 1) != 0)

    return response_sum_p_removed, rel_sample_responses_p_removed


def make_child_sum_ratio(predicted_response_sum_p_removed,
                         true_response_sum_p_removed):
    child_sum_ratio = np.divide(predicted_response_sum_p_removed,
                                true_response_sum_p_removed,
                                out=np.full_like(
                                    predicted_response_sum_p_removed, np.nan),
                                where=true_response_sum_p_removed != 0)
    return child_sum_ratio


def make_norm_abs_diff(diff_test_ref):

    norm_abs_diff = np.linalg.norm(diff_test_ref, axis=1)
    return norm_abs_diff


def make_cosine_similarity(predicted_sample_responses_p_removed,
                           true_sample_responses_p_removed):
    denominator = (
        np.linalg.norm(predicted_sample_responses_p_removed, axis=1) *
        np.linalg.norm(true_sample_responses_p_removed, axis=1))

    cosine_similarity = np.divide(np.sum(predicted_sample_responses_p_removed *
                                         true_sample_responses_p_removed,
                                         axis=1),
                                  denominator,
                                  out=np.full_like(denominator, np.nan),
                                  where=denominator != 0)
    return cosine_similarity


def make_norm_rel_diff(diff_test_ref, true_sample_responses_p_removed):
    norm_rel_diff = np.sqrt(
        np.nansum(np.divide(diff_test_ref,
                            true_sample_responses_p_removed,
                            out=np.full_like(diff_test_ref, np.nan),
                            where=true_sample_responses_p_removed != 0)**2,
                  axis=1))
    return norm_rel_diff


def create_error_measures(response_sum_p_removed,
                          rel_sample_responses_p_removed):
    '''
    Formulate different measures of error between predicted
    and ground-truth datasets.
    '''
    err_measures = {}

    diff_test_ref = rel_sample_responses_p_removed[
        'predicted'] - rel_sample_responses_p_removed['true']

    err_measures['child_sum_ratio'] = make_child_sum_ratio(
        response_sum_p_removed['predicted'], response_sum_p_removed['true'])
    err_measures['norm_abs_diff'] = make_norm_abs_diff(diff_test_ref)
    err_measures['cosine_similarity'] = make_cosine_similarity(
        rel_sample_responses_p_removed['predicted'],
        rel_sample_responses_p_removed['true'])
    err_measures['norm_rel_diff'] = make_norm_rel_diff(
        diff_test_ref, rel_sample_responses_p_removed['true'])

    return err_measures
