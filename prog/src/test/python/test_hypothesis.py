import math

import pytest
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
            assert len(set(row)) == 16, (
                f"S-box row {i} with seed {seed} has only {len(set(row))} "
                f"distinct values, expected 16"
            )
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
            assert score >= 0, f"LP_MAX score {score} is negative (seed={seed})"
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
            assert score >= 0, f"DP_MAX score {score} is negative (seed={seed})"
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
        assert stats.maxEver >= best_in_pop, (
            f"maxEver ({stats.maxEver}) < max(bestPopulationScores) "
            f"({best_in_pop}) with seed={seed}"
        )
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
                assert math.isfinite(score), (
                    f"Non-finite score {score} for criterion {criterion}"
                )
        finally:
            search.close()
