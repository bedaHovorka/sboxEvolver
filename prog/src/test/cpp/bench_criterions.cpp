#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"
#include "main.h"

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

TEST_CASE("Criterion benchmarks", "[benchmark]") {
    intVector sbox = {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7};

    BENCHMARK("computeLPMax 4x4") {
        return computeLPMax(sbox);
    };

    BENCHMARK("computeDPMax 4x4") {
        return computeDPMax(sbox);
    };

    BENCHMARK("computeBranchingFactor 4x4") {
        return computeBranchingFactor(sbox);
    };

    BENCHMARK("isBijective 4x4") {
        return isBijective(sbox);
    };

    BENCHMARK("computePolynomial 4x4") {
        return computePolynomial(sbox);
    };
}

TEST_CASE("GA step benchmark", "[benchmark]") {
    BENCHMARK("GA 1 generation 4x4 PERMUTATION LP_MAX") {
        TGenome genome = createGenome(PERMUTATION, LP_MAX, 3, 1, 4, 4);
        TSearching searching = newGaSearching(genome, 100, 1, 0.1f, 0.9f, false);
        processSearching(searching, GENERATION, 42);
        closeSearching(searching);
        closeGenome(genome);
    };
}
