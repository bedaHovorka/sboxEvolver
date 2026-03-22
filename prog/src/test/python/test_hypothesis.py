import math

from assertpy import assert_that
from hypothesis import given, strategies as st, settings

from evolution import (
    Genome,
    ChromozomeType,
    CriterionFunction,
    Searching,
    TerminationCondition,
)


@given(seed=st.integers(min_value=0, max_value=2**31 - 1))
@settings(max_examples=20, deadline=30000)
def test_bijective_permutation_always_bijective(seed):
    """After GA evolution with bijective=True, all S-box outputs must be permutations."""
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, seed)
        stats = search.statistics()
        for i in range(stats.nBestGenomes):
            row = stats.bestPopulationOutputs[i]
            assert_that(set(row)).described_as(
                f"S-box row {i} with seed {seed}"
            ).is_length(16)
    finally:
        search.close()


@given(seed=st.integers(min_value=0, max_value=2**31 - 1))
@settings(max_examples=20, deadline=30000)
def test_lp_max_scores_non_negative(seed):
    """LP_MAX fitness scores should always be non-negative."""
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, seed)
        stats = search.statistics()
        for score in stats.bestPopulationScores:
            assert_that(score).described_as(
                f"LP_MAX score (seed={seed})"
            ).is_greater_than_or_equal_to(0)
    finally:
        search.close()


@given(seed=st.integers(min_value=0, max_value=2**31 - 1))
@settings(max_examples=20, deadline=30000)
def test_dp_max_scores_non_negative(seed):
    """DP_MAX fitness scores should always be non-negative."""
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.DP_MAX, True, 4, 4)
    search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, seed)
        stats = search.statistics()
        for score in stats.bestPopulationScores:
            assert_that(score).described_as(
                f"DP_MAX score (seed={seed})"
            ).is_greater_than_or_equal_to(0)
    finally:
        search.close()


@given(seed=st.integers(min_value=0, max_value=2**31 - 1))
@settings(max_examples=20, deadline=30000)
def test_statistics_maxever_consistency(seed):
    """maxEver must be >= the maximum of bestPopulationScores."""
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, seed)
        stats = search.statistics()
        best_in_pop = max(stats.bestPopulationScores)
        assert_that(stats.maxEver).described_as(
            f"maxEver vs max(bestPopulationScores) (seed={seed})"
        ).is_greater_than_or_equal_to(best_in_pop)
    finally:
        search.close()


def test_all_criterion_functions_finite():
    """All 7 criterion functions should produce finite fitness scores."""
    criteria = [
        CriterionFunction.BENT_AND_MOSAC,
        CriterionFunction.LP_MAX,
        CriterionFunction.DP_MAX,
        CriterionFunction.BIJECTIVE_SCORE,
        CriterionFunction.SAC,
        CriterionFunction.BF,
        CriterionFunction.POLYNOMIAL_DEGREE,
    ]
    for criterion in criteria:
        genome = Genome(ChromozomeType.PERMUTATION, criterion, True, 4, 4)
        search = Searching("Ga", genome, 20, 3, 0.1, 0.9, False)
        try:
            search.process(TerminationCondition.GENERATION.ordinal, 42)
            stats = search.statistics()
            for score in stats.bestPopulationScores:
                assert_that(math.isfinite(score)).described_as(
                    f"score {score} for criterion {criterion}"
                ).is_true()
        finally:
            search.close()
