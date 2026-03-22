#include "catch.hpp"
#include "main.h"
#include <cstring>
#include <cmath>

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

// ---------------------------------------------------------------------------
// ContingentTable creation and positions
// ---------------------------------------------------------------------------
TEST_CASE("ContingentTable creation and positions", "[eda]") {
    BinaryCT table(3, 5);

    REQUIRE(table.getIPosition() == 3);
    REQUIRE(table.getJPosition() == 5);
}

// ---------------------------------------------------------------------------
// ContingentTable record and get
// ---------------------------------------------------------------------------
TEST_CASE("ContingentTable record and get", "[eda]") {
    BinaryCT table(0, 1);

    // Record some observations
    table.record(0, 0);
    table.record(0, 0);
    table.record(0, 1);
    table.record(1, 0);
    table.record(1, 1);
    table.record(1, 1);
    table.record(1, 1);

    REQUIRE(table.get(0, 0) == 2);
    REQUIRE(table.get(0, 1) == 1);
    REQUIRE(table.get(1, 0) == 1);
    REQUIRE(table.get(1, 1) == 3);
}

// ---------------------------------------------------------------------------
// ContingentTable chiTest with dependent data
// ---------------------------------------------------------------------------
TEST_CASE("ContingentTable chiTest detects dependent data", "[eda]") {
    BinaryCT table(0, 1);

    // Strongly dependent: (0,0) and (1,1) only
    for (int i = 0; i < 50; i++) {
        table.record(0, 0);
        table.record(1, 1);
    }

    // onesCounts[i] = number of individuals with bit i = 1
    // For position 0: 50 have bit=0, 50 have bit=1 -> onesCounts[0] = 50
    // For position 1: 50 have bit=0, 50 have bit=1 -> onesCounts[1] = 50
    int onesCounts[2] = {50, 50};
    int popSize = 100;

    bool dependent = table.chiTest(onesCounts, popSize);
    REQUIRE(dependent == true);
    REQUIRE(table.getTestResult() > CHI_ON_DF1_AT95);
}

// ---------------------------------------------------------------------------
// ContingentTable chiTest with independent data
// ---------------------------------------------------------------------------
TEST_CASE("ContingentTable chiTest with independent data", "[eda]") {
    BinaryCT table(0, 1);

    // Uniform distribution: all four cells equally populated
    for (int i = 0; i < 25; i++) {
        table.record(0, 0);
        table.record(0, 1);
        table.record(1, 0);
        table.record(1, 1);
    }

    // onesCounts[0] = 50 (half have bit 0 = 1), onesCounts[1] = 50
    int onesCounts[2] = {50, 50};
    int popSize = 100;

    bool dependent = table.chiTest(onesCounts, popSize);
    REQUIRE(dependent == false);
    REQUIRE(table.getTestResult() <= CHI_ON_DF1_AT95);
}

// ---------------------------------------------------------------------------
// ContingentTable comparison operator
// ---------------------------------------------------------------------------
TEST_CASE("ContingentTable comparison operator", "[eda]") {
    // Both tables use positions 0 and 1 so onesCounts arrays are straightforward
    BinaryCT tableStrong(0, 1);
    BinaryCT tableWeak(0, 1);

    // Strong dependency: only (0,0) and (1,1)
    for (int i = 0; i < 50; i++) {
        tableStrong.record(0, 0);
        tableStrong.record(1, 1);
    }

    // Weak/no dependency: uniform distribution
    for (int i = 0; i < 25; i++) {
        tableWeak.record(0, 0);
        tableWeak.record(0, 1);
        tableWeak.record(1, 0);
        tableWeak.record(1, 1);
    }

    int onesCounts[2] = {50, 50};
    tableStrong.chiTest(onesCounts, 100);
    tableWeak.chiTest(onesCounts, 100);

    REQUIRE(tableStrong > tableWeak);
    REQUIRE_FALSE(tableWeak > tableStrong);
}

// ---------------------------------------------------------------------------
// EDA BMDA via C API
// ---------------------------------------------------------------------------
TEST_CASE("EDA BMDA search via C API", "[eda]") {
    // createGenome(BINARY, DP_MAX, 2, inputs=4, outputs=4)
    TGenome genome = createGenome(BINARY, DP_MAX, 2, 4, 4);
    REQUIRE_FALSE(hasError());
    REQUIRE(genome.ptr != NULL);

    // BMDA variant (true)
    TSearching searching = newEdaSearching(genome, 30, 5, true);
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
// EDA UMDA via C API
// ---------------------------------------------------------------------------
TEST_CASE("EDA UMDA search via C API", "[eda]") {
    TGenome genome = createGenome(BINARY, DP_MAX, 2, 4, 4);
    REQUIRE_FALSE(hasError());
    REQUIRE(genome.ptr != NULL);

    // UMDA variant (false)
    TSearching searching = newEdaSearching(genome, 30, 5, false);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 55);
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
