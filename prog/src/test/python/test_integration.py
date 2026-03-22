from assertpy import assert_that

from evolution import (
    Genome,
    ChromozomeType,
    CriterionFunction,
    Searching,
    TerminationCondition,
)


def test_vega_produces_valid_criterion_values():
    """VEGA with multiple criteria should produce non-zero criterion columns."""
    criterions = [
        CriterionFunction.LP_MAX,
        CriterionFunction.DP_MAX,
        CriterionFunction.SAC,
    ]
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Vega", genome, 30, 5, 0.1, 0.9, criterions)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, 42)
        stats = search.statistics()

        assert_that(stats.nBestGenomes).is_greater_than(0)

        # Check that the criterion columns used have at least some non-zero values
        for crit in criterions:
            col_idx = crit.ordinal
            col_values = [
                stats.bestPopulationCriterionsValues[i][col_idx]
                for i in range(stats.nBestGenomes)
            ]
            assert_that(any(v != 0.0 for v in col_values)).is_true()
    finally:
        search.close()


def test_spea_produces_valid_criterion_values():
    """SPEA with multiple criteria should produce non-zero criterion columns."""
    criterions = [
        CriterionFunction.LP_MAX,
        CriterionFunction.DP_MAX,
    ]
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Spea", genome, 30, 5, 0.1, 0.9, criterions)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, 42)
        stats = search.statistics()

        assert_that(stats.nBestGenomes).is_greater_than(0)

        for crit in criterions:
            col_idx = crit.ordinal
            col_values = [
                stats.bestPopulationCriterionsValues[i][col_idx]
                for i in range(stats.nBestGenomes)
            ]
            assert_that(any(v != 0.0 for v in col_values)).is_true()
    finally:
        search.close()


def test_parallel_searching_completes():
    """Parallel searching with 4 GA instances should all complete at gen 5."""
    searches = []
    try:
        for _ in range(4):
            genome = Genome(
                ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4
            )
            search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
            searches.append(search)

        Searching.parallelSearching(searches, TerminationCondition.GENERATION)

        for i, search in enumerate(searches):
            stats = search.statistics()
            assert_that(stats.generation).described_as(
                f"search[{i}] generation"
            ).is_equal_to(5)
    finally:
        for search in searches:
            search.close()


def test_deterministic_seed_produces_same_result():
    """Running the same search configuration with the same seed twice should give identical maxEver."""
    seed = 12345
    results = []

    for _ in range(2):
        genome = Genome(
            ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4
        )
        search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
        try:
            search.process(TerminationCondition.GENERATION.ordinal, seed)
            stats = search.statistics()
            results.append(stats.maxEver)
        finally:
            search.close()

    assert_that(results[0]).is_equal_to(results[1])


def test_all_algorithm_types_complete():
    """Each supported algorithm type should complete a short run without error."""
    completed = []

    # GA
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Ga", genome, 20, 3, 0.1, 0.9, False)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, 0)
        completed.append("Ga")
    finally:
        search.close()

    # EDA BMDA
    genome = Genome(ChromozomeType.BINARY, CriterionFunction.LP_MAX, 4, 4)
    search = Searching("Eda", genome, 20, 3, True)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, 0)
        completed.append("Eda_BMDA")
    finally:
        search.close()

    # EDA UMDA
    genome = Genome(ChromozomeType.BINARY, CriterionFunction.LP_MAX, 4, 4)
    search = Searching("Eda", genome, 20, 3, False)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, 0)
        completed.append("Eda_UMDA")
    finally:
        search.close()

    # ParallelRandom
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("ParallelRandom", genome, 20, 3, 0.1)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, 0)
        completed.append("ParallelRandom")
    finally:
        search.close()

    # Random
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Random", genome, 10, 3)
    try:
        search.process(TerminationCondition.GENERATION.ordinal, 0)
        completed.append("Random")
    finally:
        search.close()

    assert_that(completed).is_length(5)
    assert_that(completed).contains("Ga", "Eda_BMDA", "Eda_UMDA", "ParallelRandom", "Random")
