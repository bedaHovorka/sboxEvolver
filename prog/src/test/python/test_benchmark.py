import pytest
from assertpy import assert_that

from evolution import (
    Genome,
    ChromozomeType,
    CriterionFunction,
    Searching,
    TerminationCondition,
)


@pytest.mark.benchmark
def test_benchmark_ga_permutation_4x4(benchmark):
    """Benchmark a full GA run with permutation genome (pop=50, 10 gens)."""

    def run_ga():
        genome = Genome(
            ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4
        )
        search = Searching("Ga", genome, 50, 10, 0.1, 0.9, False)
        search.process(TerminationCondition.GENERATION.ordinal, 42)
        stats = search.statistics()
        search.close()
        return stats.maxEver

    result = benchmark(run_ga)
    assert_that(result).is_greater_than_or_equal_to(0)


@pytest.mark.benchmark
def test_benchmark_parallel_random_cgp(benchmark):
    """Benchmark ParallelRandom with CGP genome (pop=50, 10 gens)."""

    def run_parallel_random():
        genome = Genome(
            ChromozomeType.CGP, CriterionFunction.LP_MAX, 4, 4, 4, 3, 6
        )
        search = Searching("ParallelRandom", genome, 50, 10, 0.1)
        search.process(TerminationCondition.GENERATION.ordinal, 42)
        stats = search.statistics()
        search.close()
        return stats.maxEver

    result = benchmark(run_parallel_random)
    assert_that(result).is_greater_than_or_equal_to(0)


@pytest.mark.benchmark
def test_benchmark_eda_binary(benchmark):
    """Benchmark EDA (BMDA) with binary genome (pop=50, 10 gens)."""

    def run_eda():
        genome = Genome(ChromozomeType.BINARY, CriterionFunction.LP_MAX, 4, 4)
        search = Searching("Eda", genome, 50, 10, True)
        search.process(TerminationCondition.GENERATION.ordinal, 42)
        stats = search.statistics()
        search.close()
        return stats.maxEver

    result = benchmark(run_eda)
    assert_that(result).is_greater_than_or_equal_to(0)


@pytest.mark.benchmark
def test_benchmark_statistics_retrieval(benchmark):
    """Benchmark the statistics() call on an already-completed search."""
    genome = Genome(
        ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4
    )
    search = Searching("Ga", genome, 50, 10, 0.1, 0.9, False)
    search.process(TerminationCondition.GENERATION.ordinal, 42)

    def retrieve_stats():
        stats = search.statistics()
        return stats.maxEver

    result = benchmark(retrieve_stats)
    assert_that(result).is_greater_than_or_equal_to(0)

    search.close()
