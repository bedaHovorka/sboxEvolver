#include "catch.hpp"
#include "main.h"
#include <cstring>
#include <cmath>

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

// ---------------------------------------------------------------------------
// CGP genome creation and search via C API
// ---------------------------------------------------------------------------
TEST_CASE("CGP genome creation and ParallelRandom search", "[cgp]") {
    // createGenome(CGP, LP_MAX, 5, inputs=4, outputs=4, columns=4, rows=3, mutMax=6)
    TGenome genome = createGenome(CGP, LP_MAX, 5, 4, 4, 4, 3, 6);
    REQUIRE_FALSE(hasError());
    REQUIRE(genome.ptr != NULL);

    TSearching searching = newParallelRandomSearching(genome, 30, 5, 0.1f);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 42);
    REQUIRE_FALSE(hasError());

    float scores[30];
    float criterions[30 * 8];
    int outputs[30 * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());
    REQUIRE(stats.nBestGenomes > 0);
    REQUIRE(stats.generation == 5);
    REQUIRE(stats.maxEver >= 0.0f);

    closeSearching(searching);
    closeGenome(genome);
}

// ---------------------------------------------------------------------------
// CGP outputs in valid range
// ---------------------------------------------------------------------------
TEST_CASE("CGP outputs are in valid range after search", "[cgp]") {
    TGenome genome = createGenome(CGP, LP_MAX, 5, 4, 4, 4, 3, 6);
    REQUIRE_FALSE(hasError());

    TSearching searching = newParallelRandomSearching(genome, 20, 5, 0.1f);
    processSearching(searching, GENERATION, 123);
    REQUIRE_FALSE(hasError());

    const int popsize = 20;
    const int numOutputs = 16; // 2^4
    float scores[popsize];
    float criterions[popsize * 8];
    int outputs[popsize * numOutputs];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());

    // All output values must be in [0, 2^outputs) = [0, 16)
    for (int i = 0; i < stats.nBestGenomes; i++) {
        for (int j = 0; j < numOutputs; j++) {
            int val = outputs[i * numOutputs + j];
            REQUIRE(val >= 0);
            REQUIRE(val < 16);
        }
    }

    closeSearching(searching);
    closeGenome(genome);
}

// ---------------------------------------------------------------------------
// CGP with different criteria
// ---------------------------------------------------------------------------
TEST_CASE("CGP with different criteria completes without error", "[cgp]") {
    CriterionsFitness criteria[] = { DP_MAX, SAC, BF };

    for (int c = 0; c < 3; c++) {
        SECTION(std::string("Criterion ") + std::to_string(c)) {
            TGenome genome = createGenome(CGP, criteria[c], 5, 4, 4, 4, 3, 6);
            REQUIRE_FALSE(hasError());
            REQUIRE(genome.ptr != NULL);

            TSearching searching = newParallelRandomSearching(genome, 20, 3, 0.1f);
            REQUIRE_FALSE(hasError());

            processSearching(searching, GENERATION, 77 + c);
            REQUIRE_FALSE(hasError());

            float scores[20];
            float criterionsArr[20 * 8];
            int outputs[20 * 16];
            memset(scores, 0, sizeof(scores));
            memset(criterionsArr, 0, sizeof(criterionsArr));
            memset(outputs, 0, sizeof(outputs));

            TStatistics stats = getStatisticsSearching(searching, scores, criterionsArr, outputs);
            REQUIRE_FALSE(hasError());
            REQUIRE(stats.nBestGenomes > 0);

            closeSearching(searching);
            closeGenome(genome);
        }
    }
}

// ---------------------------------------------------------------------------
// CGP symbolic regression
// ---------------------------------------------------------------------------
TEST_CASE("CGP symbolic regression completes", "[cgp]") {
    // Create expected outputs for a simple 4-input permutation
    int expectedOutputs[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // createSymbolicalRegresionGenome(CGP, expectedOutputs, 5, inputs=4, outputs=4, columns=4, rows=3, mutMax=6)
    TGenome genome = createSymbolicalRegresionGenome(CGP, expectedOutputs, 5, 4, 4, 4, 3, 6);
    REQUIRE_FALSE(hasError());
    REQUIRE(genome.ptr != NULL);

    TSearching searching = newParallelRandomSearching(genome, 20, 5, 0.1f);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 99);
    REQUIRE_FALSE(hasError());

    float scores[20];
    float criterions[20 * 8];
    int outputs[20 * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());
    REQUIRE(stats.nBestGenomes > 0);
    REQUIRE(stats.maxEver >= 0.0f);

    closeSearching(searching);
    closeGenome(genome);
}
