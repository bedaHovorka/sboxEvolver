#include "catch.hpp"
#include "main.h"

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

// ---------------------------------------------------------------------------
// SoftwareSboxGenome::testBox() - Luffa SubCrumb KAT tests
// ---------------------------------------------------------------------------
TEST_CASE("testBox() basic properties", "[softwareSbox]") {
    SoftwareSboxGenome box = SoftwareSboxGenome::testBox();

    SECTION("inputsCount is 4") {
        REQUIRE(box.inputsCount() == 4);
    }

    SECTION("outputsCount is 4") {
        REQUIRE(box.outputsCount() == 4);
    }
}

TEST_CASE("testBox() outputs are valid 4-bit values", "[softwareSbox]") {
    SoftwareSboxGenome box = SoftwareSboxGenome::testBox();
    intVector outputs = box.computeOutputs();

    REQUIRE(outputs.size() == 16);
    for (int i = 0; i < 16; i++) {
        REQUIRE(outputs[i] >= 0);
        REQUIRE(outputs[i] <= 15);
    }
}

TEST_CASE("testBox() cryptographic properties match Luffa SubCrumb", "[softwareSbox]") {
    SoftwareSboxGenome box = SoftwareSboxGenome::testBox();
    intVector outputs = box.computeOutputs();

    SECTION("DPMax = 0.25 as documented in source") {
        REQUIRE(computeDPMax(outputs) == Approx(0.25));
    }

    SECTION("LPMax = 0.25 as documented in source") {
        REQUIRE(computeLPMax(outputs) == Approx(0.25));
    }

    SECTION("branching factor is a positive integer") {
        int bf = computeBranchingFactor(outputs);
        REQUIRE(bf >= 1);
        REQUIRE(bf <= 8);
    }
}

TEST_CASE("testBox() output values are all in [0, 15]", "[softwareSbox]") {
    SoftwareSboxGenome box = SoftwareSboxGenome::testBox();
    for (int x = 0; x < 16; x++) {
        int y = box.output(x);
        REQUIRE(y >= 0);
        REQUIRE(y <= 15);
    }
}

TEST_CASE("testBox() computeCriterionsFunction LP_MAX", "[softwareSbox]") {
    SoftwareSboxGenome box = SoftwareSboxGenome::testBox();
    // LPMax = 0.25, so -log2(0.25) = 2.0
    float result = box.computeCriterionsFunction(LP_MAX);
    REQUIRE(result == Approx(2.0f));
}

TEST_CASE("testBox() computeCriterionsFunction DP_MAX", "[softwareSbox]") {
    SoftwareSboxGenome box = SoftwareSboxGenome::testBox();
    // DPMax = 0.25, so -log2(0.25) = 2.0
    float result = box.computeCriterionsFunction(DP_MAX);
    REQUIRE(result == Approx(2.0f));
}

TEST_CASE("testBox() computeCriterionsFunction BF", "[softwareSbox]") {
    SoftwareSboxGenome box = SoftwareSboxGenome::testBox();
    float result = box.computeCriterionsFunction(BF);
    REQUIRE(result >= 1.0f);
    REQUIRE(std::isfinite(result));
}

// ---------------------------------------------------------------------------
// SoftwareSboxGenome via C API
// ---------------------------------------------------------------------------
TEST_CASE("SoftwareSboxGenome via C API: createGenome and search", "[softwareSbox][capi]") {
    // SOFTWARE_IMPL needs 1 argument: bitsPerRegister
    TGenome genome = createGenome(SOFTWARE_IMPL, LP_MAX, 1, 1);
    REQUIRE(genome.ptr != NULL);

    TSearching searching = newParallelRandomSearching(genome, 10, 2, 0.1f);
    REQUIRE(searching.algorithm != NULL);

    processSearching(searching, GENERATION, 42);

    // Get statistics
    const int nBest = 10;
    float bestScores[nBest];
    float bestCriterions[nBest * 8];
    int bestOutputs[nBest * 16]; // 2^4 = 16 outputs per individual

    TStatistics stats = getStatisticsSearching(searching, bestScores, bestCriterions, bestOutputs);
    REQUIRE(std::isfinite(stats.maxEver));
    REQUIRE(stats.generation == 2);

    closeSearching(searching);
    closeGenome(genome);
}

// ---------------------------------------------------------------------------
// SoftwareSboxGenome initialization produces valid genomes
// ---------------------------------------------------------------------------
TEST_CASE("SoftwareSboxGenome random initialization produces valid outputs", "[softwareSbox]") {
    SoftwareSboxGenome genome(1);
    SoftwareSboxGenome::Init(genome);

    // After random initialization, all outputs should be in [0, 15]
    for (int x = 0; x < 16; x++) {
        int y = genome.output(x);
        REQUIRE(y >= 0);
        REQUIRE(y <= 15);
    }
}
