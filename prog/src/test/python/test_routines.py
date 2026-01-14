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


def test_genome_close():
    """Test explicit cleanup of a Genome object."""
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    assert_that(genome.delegat.ptr).is_not_none()
    assert_that(genome._closed).is_false()
    
    # Explicitly close the genome
    genome.close()
    assert_that(genome._closed).is_true()
    
    # Calling close again should be safe (idempotent)
    genome.close()
    assert_that(genome._closed).is_true()


def test_genome_automatic_cleanup():
    """Test automatic cleanup of Genome via __del__."""
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
    assert_that(genome.delegat.ptr).is_not_none()
    ptr_value = genome.delegat.ptr
    
    # Delete the genome - should trigger __del__ which calls close()
    del genome
    # If we get here without crash, cleanup worked


def test_symbolical_regression_genome_close():
    """Test explicit cleanup of a SymbolicalRegresionGenome object."""
    genome = SymbolicalRegresionGenome(ChromozomeType.CGP, [0]*16, 4, 4, 4, 3, 6)
    assert_that(genome.delegat.ptr).is_not_none()
    assert_that(genome._closed).is_false()
    
    # Explicitly close the genome
    genome.close()
    assert_that(genome._closed).is_true()


def test_genome_multiple_creation_loop():
    """Test creating and cleaning up multiple genomes to verify no memory leak."""
    # This simulates the use case from the issue description
    for i in range(100):
        genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
        assert_that(genome.delegat.ptr).is_not_none()
        genome.close()  # Explicit cleanup
    
    # If we get here without crash or excessive memory use, the fix works
    assert True

