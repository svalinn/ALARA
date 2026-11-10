import numpy as np


def prep_arrays_for_measures(parent_nuc, child_nucs, arr_Children_p_test,
                              arr_Children_p_ref):
    '''
    Takes input arrays of activation responses and removes entries containing
    the original parent nuclide. Also calculates a sum of the responses across
    each row, and normalizes the array by the vector of sums.

    :param parent_nuc (str): name of irradiated nuclide 
    :param child_nucs (iterable of str): names of all possible daughter nuclides resulting from
    the irradiation of any parent nuclide
    :param arr_Children_p_test: 2D numpy array or nested list of predicted activation responses,
    where the rows correspond to the different samples or irradiation conditions, and the columns 
    are correspond to the different daughter nuclides
    :param arr_Children_p_ref: 2D numpy array or nested list of ground-truth activation responses,
    where the rows correspond to the different samples or irradiation conditions, and the columns 
    are correspond to the different daughter nuclides
    '''
    arr_Children_p_test = np.asarray(arr_Children_p_test)
    arr_Children_p_ref = np.asarray(arr_Children_p_ref)
    parent_idx = child_nucs.index(parent_nuc)
    arr_Children_star_p_test = np.delete(arr_Children_p_test,
                                         parent_idx,
                                         axis=1)
    arr_Children_star_p_ref = np.delete(arr_Children_p_ref, parent_idx, axis=1)

    L_ref = np.sum(arr_Children_star_p_ref, axis=1)
    L_test = np.sum(arr_Children_star_p_test, axis=1)

    arr_children_star_p_test = np.divide(arr_Children_star_p_test,
                                         L_test.reshape(-1, 1),
                                         out=np.full_like(
                                             arr_Children_star_p_test, np.nan),
                                         where=L_test.reshape(-1, 1) != 0)

    arr_children_star_p_ref = np.divide(arr_Children_star_p_ref,
                                        L_ref.reshape(-1, 1),
                                        out=np.full_like(
                                            arr_Children_star_p_ref, np.nan),
                                        where=L_ref.reshape(-1, 1) != 0)

    return L_ref, L_test, arr_children_star_p_ref, arr_children_star_p_test


def create_error_measures(L_ref, L_test, arr_children_star_p_ref,
                          arr_children_star_p_test):
    '''
    Formulate different measures of error between predicted
    and ground-truth datasets.
    '''
    measure_A = np.divide(L_test,
                          L_ref,
                          out=np.full_like(L_test, np.nan),
                          where=L_ref != 0)

    diff_test_ref = arr_children_star_p_test - arr_children_star_p_ref
    measure_B = np.linalg.norm(diff_test_ref, axis=1)

    denominator = (np.linalg.norm(arr_children_star_p_test, axis=1) *
                   np.linalg.norm(arr_children_star_p_ref, axis=1))

    measure_C = np.divide(np.sum(arr_children_star_p_test *
                                 arr_children_star_p_ref,
                                 axis=1),
                          denominator,
                          out=np.full_like(denominator, np.nan),
                          where=denominator != 0)

    return measure_A, measure_B, measure_C
