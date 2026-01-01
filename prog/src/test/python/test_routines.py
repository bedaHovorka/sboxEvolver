from assertpy import assert_that

from evolution import (
    Genome,
    ChromozomeType,
    CriterionFunction,
    Searching,
    TerminationCondition,
    SymbolicalRegresionGenome,
    picleSaveTo,
    pickleLoadFrom,
)


def test_genome_creation():
    """Test creation of a Genome object."""
    genome = Genome(ChromozomeType.CGP, CriterionFunction.BENT_AND_MOSAC, 4, 4, 4, 3, 6)
    assert_that(genome.delegat.ptr).is_not_none()


def test_searching_creation():
    """Test creation of a Searching object."""
    genome = Genome(ChromozomeType.CGP, CriterionFunction.BENT_AND_MOSAC, 4, 4, 4, 3, 6)
    search = Searching("Random", genome, 10, 1)
    assert_that(search.delegat.algorithm).is_not_none()


def test_searching_run():
    """Test running a search."""
    genome = Genome(ChromozomeType.CGP, CriterionFunction.BENT_AND_MOSAC, 4, 4, 4, 3, 6)
    search = Searching("Random", genome, 10, 1)
    search.process(TerminationCondition.GENERATION.ordinal, 0)
    stats = search.statistics()
    assert_that(stats.generation).is_equal_to(1)


def test_searching_best_population():
    """Test getting the best population via statistics (bestPopulationStrings disabled)."""
    genome = Genome(ChromozomeType.CGP, CriterionFunction.BENT_AND_MOSAC, 4, 4, 4, 3, 6)
    search = Searching("Random", genome, 10, 1)
    search.process(TerminationCondition.GENERATION.ordinal, 0)
    # bestPopulationStrings() is disabled due to ctypes/PyObject* limitation
    # Instead, verify we can get population data via statistics()
    stats = search.statistics()
    assert_that(stats.bestPopulationScores).is_not_none()
    assert_that(stats.bestPopulationOutputs).is_not_none()
    assert_that(len(stats.bestPopulationScores)).is_greater_than(0)


def test_symbolical_regression_genome():
    """Test creation of a SymbolicalRegresionGenome object."""
    genome = SymbolicalRegresionGenome(ChromozomeType.CGP, [0]*16, 4, 4, 4, 3, 6)
    assert_that(genome.delegat.ptr).is_not_none()


def test_pickle_functions():
    """Test pickling and unpickling a Searching object."""
    import os
    genome = Genome(ChromozomeType.CGP, CriterionFunction.BENT_AND_MOSAC, 4, 4, 4, 3, 6)
    search = Searching("Random", genome, 10, 1)
    filename = "test_search.pkl"
    picleSaveTo(search, filename)
    assert os.path.exists(filename)
    loaded_search = pickleLoadFrom(filename)
    assert_that(loaded_search.repr).is_equal_to(search.repr)
    os.remove(filename)
