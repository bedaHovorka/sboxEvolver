#include "catch.hpp"
#include "main.h"

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

static float dummyEvaluator(GAGenome&) { return 1.0f; }

// ---------------------------------------------------------------------------
// BinarySboxGenome
// ---------------------------------------------------------------------------
TEST_CASE("BinarySboxGenome basic properties", "[boxes]") {
    BinarySboxGenome genome(2, 2, dummyEvaluator);

    SECTION("inputsCount and outputsCount") {
        REQUIRE(genome.inputsCount() == 2);
        REQUIRE(genome.outputsCount() == 2);
    }

    SECTION("genome length equals 2^inputs * outputs") {
        // 2^2 * 2 = 8 bits
        REQUIRE(genome.length() == 8);
    }

    SECTION("after initialization, output values are in valid range") {
        genome.initialize();
        for (int x = 0; x < 4; x++) {
            int y = genome.output(x);
            REQUIRE(y >= 0);
            REQUIRE(y <= 3);
        }
    }
}

// ---------------------------------------------------------------------------
// PermutationSboxGenome
// ---------------------------------------------------------------------------
TEST_CASE("PermutationSboxGenome basic properties", "[boxes]") {
    PermutationSboxGenome genome(true, 4, 4, dummyEvaluator);

    SECTION("inputsCount and outputsCount") {
        REQUIRE(genome.inputsCount() == 4);
        REQUIRE(genome.outputsCount() == 4);
    }

    SECTION("after Init, output is bijective") {
        PermutationSboxGenome::Init(genome);
        intVector outputs = genome.computeOutputs();
        REQUIRE(isBijective(outputs) == true);
    }

    SECTION("manual identity: gene(i)=i, output(5)==5") {
        for (int i = 0; i < 16; i++) {
            genome.gene(i, i);
        }
        REQUIRE(genome.output(5) == 5);
    }
}

TEST_CASE("PermutationSboxGenome non-bijective variant", "[boxes]") {
    PermutationSboxGenome genome(false, 4, 2, dummyEvaluator);
    REQUIRE(genome.inputsCount() == 4);
    REQUIRE(genome.outputsCount() == 2);

    PermutationSboxGenome::Init(genome);
    // All output values should be in [0, 3] (2^2 - 1)
    for (int x = 0; x < 16; x++) {
        int y = genome.output(x);
        REQUIRE(y >= 0);
        REQUIRE(y <= 3);
    }
}

// ---------------------------------------------------------------------------
// Sbox::computeCriterionsFunction dispatch
// ---------------------------------------------------------------------------
TEST_CASE("computeCriterionsFunction on identity PermutationSboxGenome", "[boxes]") {
    PermutationSboxGenome genome(true, 4, 4, dummyEvaluator);
    for (int i = 0; i < 16; i++) {
        genome.gene(i, i);
    }

    SECTION("LP_MAX: identity is fully linear, -log2(1.0) = 0.0") {
        float result = genome.computeCriterionsFunction(LP_MAX);
        REQUIRE(result == Approx(0.0f));
    }

    SECTION("DP_MAX: identity has DP_MAX=1.0, -log2(1.0) = 0.0") {
        float result = genome.computeCriterionsFunction(DP_MAX);
        REQUIRE(result == Approx(0.0f));
    }

    SECTION("NONE_CRITERION throws runtime_error") {
        REQUIRE_THROWS_AS(genome.computeCriterionsFunction(NONE_CRITERION), std::runtime_error);
    }

    SECTION("LAGRANGE throws runtime_error (not implemented)") {
        REQUIRE_THROWS_AS(genome.computeCriterionsFunction(LAGRANGE), std::runtime_error);
    }
}

TEST_CASE("computeCriterionsFunction returns finite values for known permutation", "[boxes]") {
    PermutationSboxGenome genome(true, 4, 4, dummyEvaluator);
    // Set to reversal permutation: {15, 14, 13, ..., 0}
    for (int i = 0; i < 16; i++) {
        genome.gene(i, 15 - i);
    }

    float lpMax = genome.computeCriterionsFunction(LP_MAX);
    float dpMax = genome.computeCriterionsFunction(DP_MAX);
    REQUIRE(std::isfinite(lpMax));
    REQUIRE(std::isfinite(dpMax));
}

TEST_CASE("computeCriterionsFunction BF on identity", "[boxes]") {
    PermutationSboxGenome genome(true, 4, 4, dummyEvaluator);
    for (int i = 0; i < 16; i++) {
        genome.gene(i, i);
    }
    float bf = genome.computeCriterionsFunction(BF);
    REQUIRE(bf == Approx(2.0f));
}

TEST_CASE("computeCriterionsFunction BIJECTIVE_SCORE on identity", "[boxes]") {
    PermutationSboxGenome genome(true, 4, 4, dummyEvaluator);
    for (int i = 0; i < 16; i++) {
        genome.gene(i, i);
    }
    float score = genome.computeCriterionsFunction(BIJECTIVE_SCORE);
    REQUIRE(score > 0.0f);
}

TEST_CASE("computeCriterionsFunction SAC on identity", "[boxes]") {
    PermutationSboxGenome genome(true, 4, 4, dummyEvaluator);
    for (int i = 0; i < 16; i++) {
        genome.gene(i, i);
    }
    // SAC returns strictAvelancheCriterion(...) + 1
    // Identity fails order 0, so SAC returns -1 + 1 = 0
    float sac = genome.computeCriterionsFunction(SAC);
    REQUIRE(sac == Approx(0.0f));
}

TEST_CASE("computeCriterionsFunction POLYNOMIAL_DEGREE on identity", "[boxes]") {
    PermutationSboxGenome genome(true, 4, 4, dummyEvaluator);
    for (int i = 0; i < 16; i++) {
        genome.gene(i, i);
    }
    float deg = genome.computeCriterionsFunction(POLYNOMIAL_DEGREE);
    REQUIRE(deg >= 0.0f);
}
