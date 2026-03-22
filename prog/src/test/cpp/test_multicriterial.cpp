#include "catch.hpp"
#include "main.h"
#include <cstring>
#include <cmath>

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

// ---------------------------------------------------------------------------
// VEGA with 3 criteria
// ---------------------------------------------------------------------------
TEST_CASE("VEGA with 3 criteria on PERMUTATION genome", "[multicriterial]") {
    TGenome genome = createGenome(PERMUTATION, NONE_CRITERION, 3, true, 4, 4);
    REQUIRE_FALSE(hasError());
    REQUIRE(genome.ptr != NULL);

    CriterionsFitness crits[] = { LP_MAX, DP_MAX, SAC };
    TSearching searching = newVegaSearching(genome, 30, 5, 0.1f, 0.9f, 3, crits);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 42);
    REQUIRE_FALSE(hasError());

    const int popsize = 30;
    float scores[popsize];
    float criterions[popsize * 8];
    int outputs[popsize * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());
    REQUIRE(stats.nBestGenomes > 0);
    REQUIRE(std::isfinite(stats.maxEver));
    REQUIRE(stats.generation == 5);

    closeSearching(searching);
    closeGenome(genome);
}

// ---------------------------------------------------------------------------
// SPEA with 2 criteria
// ---------------------------------------------------------------------------
TEST_CASE("SPEA with 2 criteria on PERMUTATION genome", "[multicriterial]") {
    TGenome genome = createGenome(PERMUTATION, NONE_CRITERION, 3, true, 4, 4);
    REQUIRE_FALSE(hasError());
    REQUIRE(genome.ptr != NULL);

    CriterionsFitness crits[] = { LP_MAX, DP_MAX };
    TSearching searching = newSpeaSearching(genome, 30, 5, 0.1f, 0.9f, 2, crits);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 55);
    REQUIRE_FALSE(hasError());

    const int popsize = 30;
    float scores[popsize];
    float criterions[popsize * 8];
    int outputs[popsize * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());
    REQUIRE(stats.nBestGenomes > 0);
    REQUIRE(std::isfinite(stats.maxEver));

    closeSearching(searching);
    closeGenome(genome);
}

// ---------------------------------------------------------------------------
// VEGA with BINARY genome type
// ---------------------------------------------------------------------------
TEST_CASE("VEGA with BINARY genome completes", "[multicriterial]") {
    TGenome genome = createGenome(BINARY, NONE_CRITERION, 2, 4, 4);
    REQUIRE_FALSE(hasError());
    REQUIRE(genome.ptr != NULL);

    CriterionsFitness crits[] = { LP_MAX, SAC };
    TSearching searching = newVegaSearching(genome, 30, 5, 0.1f, 0.9f, 2, crits);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 77);
    REQUIRE_FALSE(hasError());

    const int popsize = 30;
    float scores[popsize];
    float criterions[popsize * 8];
    int outputs[popsize * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());
    REQUIRE(stats.nBestGenomes > 0);

    closeSearching(searching);
    closeGenome(genome);
}

// ---------------------------------------------------------------------------
// Multi-objective stats arrays have data for specified criteria
// ---------------------------------------------------------------------------
TEST_CASE("VEGA bestPopulationCriterionsValues has data for specified criteria", "[multicriterial]") {
    TGenome genome = createGenome(PERMUTATION, NONE_CRITERION, 3, true, 4, 4);
    REQUIRE_FALSE(hasError());

    CriterionsFitness crits[] = { LP_MAX, DP_MAX, SAC };
    TSearching searching = newVegaSearching(genome, 30, 5, 0.1f, 0.9f, 3, crits);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 88);
    REQUIRE_FALSE(hasError());

    const int popsize = 30;
    float scores[popsize];
    float criterions[popsize * 8];
    int outputs[popsize * 16];
    memset(scores, 0, sizeof(scores));
    memset(outputs, 0, sizeof(outputs));
    // Initialize criterions with NaN sentinel to detect which columns were actually written
    for (int i = 0; i < popsize * 8; i++) criterions[i] = NAN;

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());

    // Check that criterion columns for LP_MAX(1), DP_MAX(2), SAC(4) were populated
    // Initialize with NaN sentinel so we can distinguish "written as 0.0" from "never written"
    // Here we verify the values are finite (not NaN/inf), confirming the implementation wrote them
    for (int i = 0; i < stats.nBestGenomes; i++) {
        REQUIRE(std::isfinite(criterions[i * 8 + LP_MAX]));
        REQUIRE(std::isfinite(criterions[i * 8 + DP_MAX]));
        REQUIRE(std::isfinite(criterions[i * 8 + SAC]));
    }

    closeSearching(searching);
    closeGenome(genome);
}

// ---------------------------------------------------------------------------
// SPEA criterion values are valid floats (no NaN or inf)
// ---------------------------------------------------------------------------
TEST_CASE("SPEA criterion values are valid floats", "[multicriterial]") {
    TGenome genome = createGenome(PERMUTATION, NONE_CRITERION, 3, true, 4, 4);
    REQUIRE_FALSE(hasError());

    CriterionsFitness crits[] = { LP_MAX, DP_MAX };
    TSearching searching = newSpeaSearching(genome, 30, 5, 0.1f, 0.9f, 2, crits);
    REQUIRE_FALSE(hasError());

    processSearching(searching, GENERATION, 99);
    REQUIRE_FALSE(hasError());

    const int popsize = 30;
    float scores[popsize];
    float criterions[popsize * 8];
    int outputs[popsize * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));

    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    REQUIRE_FALSE(hasError());

    // Verify no NaN or inf in the criterion values for all individuals
    for (int i = 0; i < stats.nBestGenomes; i++) {
        for (int j = 0; j < 8; j++) {
            float val = criterions[i * 8 + j];
            REQUIRE(std::isfinite(val));
            REQUIRE_FALSE(std::isnan(val));
        }
    }

    // Also verify scores are valid
    for (int i = 0; i < stats.nBestGenomes; i++) {
        REQUIRE(std::isfinite(scores[i]));
    }

    closeSearching(searching);
    closeGenome(genome);
}
