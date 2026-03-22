import pytest
from evolution import (
    Genome,
    ChromozomeType,
    CriterionFunction,
    Searching,
    TerminationCondition,
)


@pytest.fixture
def permutation_genome_4x4():
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    yield genome
    genome.close()


@pytest.fixture
def binary_genome_4x4():
    genome = Genome(ChromozomeType.BINARY, CriterionFunction.LP_MAX, 4, 4)
    yield genome
    genome.close()


@pytest.fixture
def cgp_genome_4x4():
    genome = Genome(ChromozomeType.CGP, CriterionFunction.LP_MAX, 4, 4, 4, 3, 6)
    yield genome
    genome.close()


@pytest.fixture
def software_genome():
    genome = Genome(ChromozomeType.SOFTWARE_IMPL, CriterionFunction.LP_MAX, 1)
    yield genome
    genome.close()


@pytest.fixture
def short_ga_search():
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
    yield search
    search.close()


@pytest.fixture
def completed_search():
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    search = Searching("Ga", genome, 20, 5, 0.1, 0.9, False)
    search.process(TerminationCondition.GENERATION.ordinal, 42)
    yield search
    search.close()
