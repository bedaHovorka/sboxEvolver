/*
 * Valgrind memory leak test harness for libsboxevolution
 *
 * Exercises the full lifecycle of genome creation, search algorithms,
 * statistics retrieval, and cleanup to verify RAII and manual cleanup
 * paths are leak-free.
 */
#include "main.h"
#include <cstdio>
#include <cstring>

static void testGaPermutation() {
    printf("=== Test: GA with Permutation genome ===\n");
    TGenome genome = createGenome(PERMUTATION, LP_MAX, 3, 1, 4, 4);
    TSearching searching = newGaSearching(genome, 50, 10, 0.1f, 0.9f, false);
    processSearching(searching, GENERATION, 42);

    float scores[50];
    float criterions[50 * 8];
    int outputs[50 * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));
    TStatistics stats = getStatisticsSearching(searching, scores, criterions, outputs);
    printf("  maxEver=%.4f generations=%d\n", stats.maxEver, stats.generation);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testGaBinary() {
    printf("=== Test: GA with Binary genome ===\n");
    TGenome genome = createGenome(BINARY, SAC, 2, 4, 4);
    TSearching searching = newGaSearching(genome, 30, 5, 0.1f, 0.9f, true);
    processSearching(searching, GENERATION, 123);

    float scores[30];
    float criterions[30 * 8];
    int outputs[30 * 16];
    memset(scores, 0, sizeof(scores));
    memset(criterions, 0, sizeof(criterions));
    memset(outputs, 0, sizeof(outputs));
    getStatisticsSearching(searching, scores, criterions, outputs);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testEdaBmda() {
    printf("=== Test: EDA (BMDA) with Binary genome ===\n");
    TGenome genome = createGenome(BINARY, DP_MAX, 2, 4, 4);
    TSearching searching = newEdaSearching(genome, 30, 5, true);
    processSearching(searching, GENERATION, 99);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testEdaUmda() {
    printf("=== Test: EDA (UMDA) with Binary genome ===\n");
    TGenome genome = createGenome(BINARY, LP_MAX, 2, 4, 4);
    TSearching searching = newEdaSearching(genome, 30, 5, false);
    processSearching(searching, GENERATION, 77);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testParallelRandom() {
    printf("=== Test: ParallelRandom with Permutation genome ===\n");
    TGenome genome = createGenome(PERMUTATION, DP_MAX, 3, 1, 4, 4);
    TSearching searching = newParallelRandomSearching(genome, 20, 5, 0.1f);
    processSearching(searching, GENERATION, 55);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testRandomSearch() {
    printf("=== Test: Random search ===\n");
    TGenome genome = createGenome(PERMUTATION, SAC, 3, 1, 4, 4);
    TSearching searching = newRandomSearching(genome, 10, 5);
    processSearching(searching, GENERATION, 33);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testVega() {
    printf("=== Test: VEGA multi-objective ===\n");
    TGenome genome = createGenome(PERMUTATION, NONE_CRITERION, 3, 1, 4, 4);
    CriterionsFitness criterions[] = {LP_MAX, DP_MAX, SAC};
    TSearching searching = newVegaSearching(genome, 30, 5, 0.1f, 0.9f, 3, criterions);
    processSearching(searching, GENERATION, 44);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testSpea() {
    printf("=== Test: SPEA multi-objective ===\n");
    TGenome genome = createGenome(PERMUTATION, NONE_CRITERION, 3, 1, 4, 4);
    CriterionsFitness criterions[] = {LP_MAX, DP_MAX};
    TSearching searching = newSpeaSearching(genome, 30, 5, 0.1f, 0.9f, 2, criterions);
    processSearching(searching, GENERATION, 66);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testCgpGenome() {
    printf("=== Test: CGP genome ===\n");
    TGenome genome = createGenome(CGP, LP_MAX, 5, 4, 4, 4, 3, 6);
    TSearching searching = newParallelRandomSearching(genome, 20, 5, 0.1f);
    processSearching(searching, GENERATION, 88);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testSoftwareImplGenome() {
    printf("=== Test: SOFTWARE_IMPL genome ===\n");
    TGenome genome = createGenome(SOFTWARE_IMPL, LP_MAX, 1, 1);
    TSearching searching = newParallelRandomSearching(genome, 20, 5, 0.1f);
    processSearching(searching, GENERATION, 11);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

static void testSymbolicRegression() {
    printf("=== Test: Symbolic regression genome ===\n");
    int expectedOutputs[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    TGenome genome = createSymbolicalRegresionGenome(CGP, expectedOutputs, 5, 4, 4, 4, 3, 6);
    TSearching searching = newParallelRandomSearching(genome, 20, 5, 0.1f);
    processSearching(searching, GENERATION, 22);

    closeSearching(searching);
    closeGenome(genome);
    printf("  PASSED\n\n");
}

int main() {
    printf("Valgrind Memory Leak Test Harness\n");
    printf("=================================\n\n");

    initialization();

    testGaPermutation();
    testGaBinary();
    testEdaBmda();
    testEdaUmda();
    testParallelRandom();
    testRandomSearch();
    testVega();
    testSpea();
    testCgpGenome();
    testSoftwareImplGenome();
    testSymbolicRegression();

    printf("All tests completed.\n");
    return 0;
}
