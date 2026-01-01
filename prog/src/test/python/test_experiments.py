from assertpy import assert_that

from evolution import Genome, ChromozomeType, CriterionFunction, Searching


def test_short_experiment():
    """Run a short experiment to test the evolution process."""
    # This is a simplified version of an experiment
    # Using small population and few generations to run quickly
    popSize = 10
    generations = 2
    genome = Genome(ChromozomeType.CGP, CriterionFunction.BENT_AND_MOSAC, 4, 4, 4, 3, 6)
    # Note: Ga requires 6 params: genome, popSize, nGen, pMut, pCross, elitism
    # CGP genomes don't support crossover, so pCross must be 0.0
    search = Searching("Ga", genome, popSize, generations, 0.1, 0.0, False)

    # The evolution process should complete without raising an exception.
    try:
        search.simpleEvolveAndClose()
        completed = True
    except Exception as e:
        completed = False
        print("Evolution process failed with exception: {}".format(e))

    # If it runs without error, the test is considered passed.
    assert_that(completed).is_true()


def test_binary_experiment():
    """Run a short experiment with BINARY chromozome type."""
    popSize = 10
    generations = 2
    genome = Genome(ChromozomeType.BINARY, CriterionFunction.LP_MAX, 4, 4)
    search = Searching("Ga", genome, popSize, generations, 0.1, 0.8, False)

    try:
        search.simpleEvolveAndClose()
        completed = True
    except Exception as e:
        completed = False
        print("Evolution process failed with exception: {}".format(e))

    assert_that(completed).is_true()


def test_permutation_experiment():
    """Run a short experiment with PERMUTATION chromozome type."""
    popSize = 10
    generations = 2
    # For PERMUTATION, args are (vector_size, inputs, outputs)
    genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.DP_MAX, 16, 4, 4)
    search = Searching("Ga", genome, popSize, generations, 0.1, 0.8, False)

    try:
        search.simpleEvolveAndClose()
        completed = True
    except Exception as e:
        completed = False
        print("Evolution process failed with exception: {}".format(e))

    assert_that(completed).is_true()


def test_software_experiment():
    """Run a short experiment with SOFTWARE_IMPL chromozome type."""
    popSize = 10
    generations = 2
    # For SOFTWARE_IMPL, the argument is the size of the S-box in bytes,
    # which determines inputs and outputs (e.g., 1 byte = 4 inputs/outputs).
    genome = Genome(ChromozomeType.SOFTWARE_IMPL, CriterionFunction.SAC, 1)
    search = Searching("Ga", genome, popSize, generations, 0.1, 0.8, False)

    try:
        search.simpleEvolveAndClose()
        completed = True
    except Exception as e:
        completed = False
        print("Evolution process failed with exception: {}".format(e))

    assert_that(completed).is_true()
