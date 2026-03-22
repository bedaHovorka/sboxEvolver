#include "catch.hpp"
#include "main.h"

static struct Initializer {
    Initializer() { initialization(); }
} s_initializer;

// ---------------------------------------------------------------------------
// hammingWeight
// ---------------------------------------------------------------------------
TEST_CASE("hammingWeight computes popcount correctly", "[criterions]") {
    REQUIRE(hammingWeight(0) == 0);
    REQUIRE(hammingWeight(1) == 1);
    REQUIRE(hammingWeight(0xF) == 4);
    REQUIRE(hammingWeight(0xFF) == 8);
    REQUIRE(hammingWeight(0x80) == 1);
}

// ---------------------------------------------------------------------------
// binaryDot
// ---------------------------------------------------------------------------
TEST_CASE("binaryDot computes parity of bitwise AND", "[criterions]") {
    SECTION("both zero") {
        REQUIRE(binaryDot(0, 0) == 0);
    }
    SECTION("0xFF & 0xFF has 8 bits set - even parity") {
        REQUIRE(binaryDot(0xFF, 0xFF) == 0);
    }
    SECTION("1 & 1 has 1 bit set - odd parity") {
        REQUIRE(binaryDot(1, 1) == 1);
    }
    SECTION("3 & 3 has 2 bits set - even parity") {
        REQUIRE(binaryDot(3, 3) == 0);
    }
    SECTION("7 & 7 has 3 bits set - odd parity") {
        REQUIRE(binaryDot(7, 7) == 1);
    }
}

// ---------------------------------------------------------------------------
// highestOneBit
// ---------------------------------------------------------------------------
TEST_CASE("highestOneBit isolates the most significant set bit", "[criterions]") {
    REQUIRE(highestOneBit(0) == 0);
    REQUIRE(highestOneBit(1) == 1);
    REQUIRE(highestOneBit(5) == 4);
    REQUIRE(highestOneBit(255) == 128);
    REQUIRE(highestOneBit(256) == 256);
}

// ---------------------------------------------------------------------------
// computeLPMax
// ---------------------------------------------------------------------------
TEST_CASE("computeLPMax returns expected values for known S-boxes", "[criterions]") {
    SECTION("identity 4-bit is completely linear, LP_MAX = 1.0") {
        intVector identity;
        for (int i = 0; i < 16; i++) identity.push_back(i);
        REQUIRE(computeLPMax(identity) == Approx(1.0));
    }

    SECTION("constant zero 4-bit, LP_MAX = 1.0") {
        intVector constant(16, 0);
        REQUIRE(computeLPMax(constant) == Approx(1.0));
    }
}

// ---------------------------------------------------------------------------
// computeDPMax
// ---------------------------------------------------------------------------
TEST_CASE("computeDPMax returns expected values for known S-boxes", "[criterions]") {
    SECTION("identity 4-bit, DP_MAX = 1.0") {
        intVector identity;
        for (int i = 0; i < 16; i++) identity.push_back(i);
        REQUIRE(computeDPMax(identity) == Approx(1.0));
    }

    SECTION("constant zero 4-element, DP_MAX = 1.0") {
        intVector constant(4, 0);
        REQUIRE(computeDPMax(constant) == Approx(1.0));
    }
}

// ---------------------------------------------------------------------------
// isBijective
// ---------------------------------------------------------------------------
TEST_CASE("isBijective detects permutations vs non-permutations", "[criterions]") {
    SECTION("identity is bijective") {
        intVector id = {0, 1, 2, 3};
        REQUIRE(isBijective(id) == true);
    }
    SECTION("constant is not bijective") {
        intVector c = {0, 0, 0, 0};
        REQUIRE(isBijective(c) == false);
    }
    SECTION("reversal is bijective") {
        intVector rev = {3, 2, 1, 0};
        REQUIRE(isBijective(rev) == true);
    }
    SECTION("duplicate element is not bijective") {
        intVector dup = {0, 1, 2, 2};
        REQUIRE(isBijective(dup) == false);
    }
}

// ---------------------------------------------------------------------------
// isBidirectional
// ---------------------------------------------------------------------------
TEST_CASE("isBidirectional detects involutions", "[criterions]") {
    SECTION("identity is bidirectional") {
        intVector id = {0, 1, 2, 3};
        REQUIRE(isBidirectional(id) == true);
    }
    SECTION("pairwise swap is bidirectional") {
        intVector swap = {1, 0, 3, 2};
        REQUIRE(isBidirectional(swap) == true);
    }
    SECTION("cycle (1,2,0,3) is not bidirectional: f(f(0))=f(1)=2 != 0") {
        intVector cycle = {1, 2, 0, 3};
        REQUIRE(isBidirectional(cycle) == false);
    }
}

// ---------------------------------------------------------------------------
// computeBranchingFactor
// ---------------------------------------------------------------------------
TEST_CASE("computeBranchingFactor on identity 2-bit", "[criterions]") {
    intVector identity = {0, 1, 2, 3};
    // For identity: dx=1, x0=0: dy=1, hw(1)+hw(1)=2. Minimum across all is 2.
    REQUIRE(computeBranchingFactor(identity) == 2);
}

// ---------------------------------------------------------------------------
// computePolynomial
// ---------------------------------------------------------------------------
TEST_CASE("computePolynomial returns vector of same size", "[criterions]") {
    intVector identity = {0, 1, 2, 3};
    intVector poly = computePolynomial(identity);
    REQUIRE(poly.size() == identity.size());

    SECTION("ANF of 2-bit identity: {0,1,2,0}") {
        // polynomial[0] = fun[0] = 0
        // polynomial[1] = fun[1] ^ fun[0] = 1 ^ 0 = 1
        // polynomial[2] = fun[2] ^ fun[0] = 2 ^ 0 = 2
        // polynomial[3] = fun[3] ^ fun[2] ^ fun[1] ^ fun[0] = 3^2^1^0 = 0
        REQUIRE(poly[0] == 0);
        REQUIRE(poly[1] == 1);
        REQUIRE(poly[2] == 2);
        REQUIRE(poly[3] == 0);
    }
}

// ---------------------------------------------------------------------------
// polynomialDegrees
// ---------------------------------------------------------------------------
TEST_CASE("polynomialDegrees on polynomial of 2-bit identity", "[criterions]") {
    intVector identity = {0, 1, 2, 3};
    intVector poly = computePolynomial(identity);
    intVector degrees = polynomialDegrees(poly);
    // polynomial = {0, 1, 2, 0}
    // highestBitPosition({0,1,2,0}) = highestOneBit(1|2) = highestOneBit(3) = 2
    // So we get degrees for bit 0 (mask=1) and bit 1 (mask=2):
    // bit 0: poly[1] has mask 1 set, hammingWeight(1)=1. degree=1
    // bit 1: poly[2] has mask 2 set, hammingWeight(2)=1. degree=1
    REQUIRE(degrees.size() == 2);
    REQUIRE(degrees[0] == 1);
    REQUIRE(degrees[1] == 1);
}

// ---------------------------------------------------------------------------
// strictAvelancheCriterion
// ---------------------------------------------------------------------------
TEST_CASE("strictAvelancheCriterion on 4-bit identity", "[criterions]") {
    intVector identity;
    for (int i = 0; i < 16; i++) identity.push_back(i);
    // Identity fails order 0 SAC, so result = 0 - 1 = -1
    int result = strictAvelancheCriterion(4, 4, identity);
    REQUIRE(result == -1);
}

// ---------------------------------------------------------------------------
// bentScore
// ---------------------------------------------------------------------------
TEST_CASE("bentScore returns non-negative value", "[criterions]") {
    intVector identity;
    for (int i = 0; i < 16; i++) identity.push_back(i);
    float score = bentScore(4, 4, identity);
    REQUIRE(score >= 0.0f);
}

// ---------------------------------------------------------------------------
// bijectiveScore
// ---------------------------------------------------------------------------
TEST_CASE("bijectiveScore returns positive value for bijective function", "[criterions]") {
    intVector identity = {0, 1, 2, 3};
    float score = bijectiveScore(identity);
    REQUIRE(score > 0.0f);
}

TEST_CASE("bijectiveScore for constant is less than for bijective", "[criterions]") {
    intVector bijective = {0, 1, 2, 3};
    intVector constant = {0, 0, 0, 0};
    float bijScore = bijectiveScore(bijective);
    float constScore = bijectiveScore(constant);
    REQUIRE(bijScore > constScore);
}
