#include "catch.hpp"
#include "main.h"

#include <vector>
#include <algorithm>

using namespace stdcomb;

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

// ---------------------------------------------------------------------------
// next_combination
// ---------------------------------------------------------------------------
TEST_CASE("next_combination enumerates all C(4,2) combinations", "[combination]") {
    std::vector<int> n = {0, 1, 2, 3};
    std::vector<int> r = {0, 1};

    std::vector<std::vector<int> > results;
    results.push_back(r); // initial combination

    while (next_combination(n.begin(), n.end(), r.begin(), r.end())) {
        results.push_back(r);
    }

    REQUIRE(results.size() == 6);

    // Verify all expected combinations are present
    std::vector<std::vector<int> > expected;
    int e0[] = {0, 1}; expected.push_back(std::vector<int>(e0, e0 + 2));
    int e1[] = {0, 2}; expected.push_back(std::vector<int>(e1, e1 + 2));
    int e2[] = {0, 3}; expected.push_back(std::vector<int>(e2, e2 + 2));
    int e3[] = {1, 2}; expected.push_back(std::vector<int>(e3, e3 + 2));
    int e4[] = {1, 3}; expected.push_back(std::vector<int>(e4, e4 + 2));
    int e5[] = {2, 3}; expected.push_back(std::vector<int>(e5, e5 + 2));

    for (size_t i = 0; i < expected.size(); i++) {
        REQUIRE(results[i] == expected[i]);
    }
}

TEST_CASE("next_combination with single element r from n={0,1,2}", "[combination]") {
    std::vector<int> n = {0, 1, 2};
    std::vector<int> r = {0};

    std::vector<std::vector<int> > results;
    results.push_back(r);

    while (next_combination(n.begin(), n.end(), r.begin(), r.end())) {
        results.push_back(r);
    }

    REQUIRE(results.size() == 3);
    REQUIRE(results[0][0] == 0);
    REQUIRE(results[1][0] == 1);
    REQUIRE(results[2][0] == 2);
}

TEST_CASE("next_combination returns false when no more combinations", "[combination]") {
    std::vector<int> n = {0, 1, 2, 3};
    std::vector<int> r = {2, 3}; // last combination

    bool hasNext = next_combination(n.begin(), n.end(), r.begin(), r.end());
    REQUIRE(hasNext == false);
}

TEST_CASE("next_combination C(5,3) yields 10 combinations", "[combination]") {
    std::vector<int> n = {0, 1, 2, 3, 4};
    std::vector<int> r = {0, 1, 2};

    int count = 1; // initial
    while (next_combination(n.begin(), n.end(), r.begin(), r.end())) {
        count++;
    }
    REQUIRE(count == 10);
}

// ---------------------------------------------------------------------------
// prev_combination
// ---------------------------------------------------------------------------
TEST_CASE("prev_combination iterates backwards from last C(4,2)", "[combination]") {
    std::vector<int> n = {0, 1, 2, 3};
    std::vector<int> r = {2, 3}; // last combination

    std::vector<std::vector<int> > results;

    while (prev_combination(n.begin(), n.end(), r.begin(), r.end())) {
        results.push_back(r);
    }

    // From {2,3}, going backwards: {1,3}, {1,2}, {0,3}, {0,2}, {0,1}
    REQUIRE(results.size() == 5);

    std::vector<std::vector<int> > expected;
    int e0[] = {1, 3}; expected.push_back(std::vector<int>(e0, e0 + 2));
    int e1[] = {1, 2}; expected.push_back(std::vector<int>(e1, e1 + 2));
    int e2[] = {0, 3}; expected.push_back(std::vector<int>(e2, e2 + 2));
    int e3[] = {0, 2}; expected.push_back(std::vector<int>(e3, e3 + 2));
    int e4[] = {0, 1}; expected.push_back(std::vector<int>(e4, e4 + 2));

    for (size_t i = 0; i < expected.size(); i++) {
        REQUIRE(results[i] == expected[i]);
    }
}

TEST_CASE("prev_combination returns false at first combination", "[combination]") {
    std::vector<int> n = {0, 1, 2, 3};
    std::vector<int> r = {0, 1}; // first combination

    bool hasPrev = prev_combination(n.begin(), n.end(), r.begin(), r.end());
    REQUIRE(hasPrev == false);
}

TEST_CASE("next then prev are inverse operations", "[combination]") {
    std::vector<int> n = {0, 1, 2, 3, 4};
    std::vector<int> r = {0, 1, 2};
    std::vector<int> original = r;

    // Go forward one step
    next_combination(n.begin(), n.end(), r.begin(), r.end());
    REQUIRE(r != original);

    // Go back one step
    prev_combination(n.begin(), n.end(), r.begin(), r.end());
    REQUIRE(r == original);
}
