/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * criterions.h
 *
 *  Created on: Mar 11, 2010
 *      Author: Bedrich Hovorka
 *
 * Implementation of Criterions
 */
#ifndef CRITERIONS_H_
#define CRITERIONS_H_

#include <climits>
#include <set>
#include <map>
#include <sstream>
#include <stdexcept>
#include "combination.h"
#include "common.h"

extern "C" {
	typedef enum {
		BENT_AND_MOSAC,
		LP_MAX,
		DP_MAX,
		BIJECTIVE_SCORE,
		SAC,
		BF,
		POLYNOMIAL_DEGREE,
		MINIMAL_REALISATION,
		LAGRANGE,
		NONE_CRITERION
	} CriterionsFitness;
}

typedef std::map<CriterionsFitness, float> MultievaluationValues;

typedef std::set<CriterionsFitness> CriterionsSet;

float bentAndMosacObjective(GAGenome &);
float lpMaxFunctionObjective(GAGenome & g);
float dpMaxFunctionObjective(GAGenome & g);
float bijectiveObjective(GAGenome &g);
float sacObjective(GAGenome &g);
float branchingFactorObjective(GAGenome &g);
float noneCriterionObjective(GAGenome &g);
float symbolicalRegresionObjective(GAGenome &g);

GAGenome::Evaluator criterionsEnumToFunction(CriterionsFitness function);

using namespace stdcomb;

/*
 * SAC
 */

// test nad vymezenym polem...
inline bool privateSac(const intVector & inputs, const intVector & inputsCombinations, const int outputsCount, const intVector & function) {
	for (intVector::const_iterator i = inputs.begin(); i != inputs.end(); i++) {
		register uint sum = 0;
		for (intVector::const_iterator x = inputsCombinations.begin(); x != inputsCombinations.end(); x++) {
			sum += hammingWeight(function[*x] ^ function[*x ^ (1<<*i)]);
		}
		if (sum != outputsCount * (inputsCombinations.size() >> 1)) return false;
	}
	return true;
}

// cast testu tykajici se jednoho radu...
inline bool sac(const int order, const int inputsCount, const int outputsCount, const intVector & function) {
	intVector inputsVector;
	inputsVector.reserve(inputsCount);
	for (int i = 0; i < inputsCount; i++) inputsVector.push_back(i);

	if (order == 0) {
		intVector helper;
		helper.reserve(1<<inputsCount);
		for (int i = 0; i < 1<<inputsCount; i++) helper.push_back(i);
		return privateSac(inputsVector, helper, outputsCount, function);
	}
	assert(order > 0);

	// pripravit (inputsCount-order)-tice a nad nimy spustit privateSac
	intVector currentCombination(inputsVector.begin(), inputsVector.end()-order);

	// pro vsecny mozne posice zredukovane (inputsCount, inputs)
	do {
		// pro vsecny 2^order -> zredukovane (inputCountcombinations)
		for (int i = 0; i < 1<<order; i++) {
			intVector inputCombinations;
			// menici se zbytky
			for (int j = 0; j < 1<<(inputsCount-order); j++) {
				int r = 0;
				for (int n = 0, c = 0, z = 0; n < inputsCount; n++) {
					if (std::find(currentCombination.begin(), currentCombination.end(), n) == currentCombination.end()) {
						if (i & (1<<c)) r |= 1<<n;
						c++;
					} else {
						if (j & (1<<z)) r |= 1<<n;
						z++;
					}
				}
//				display(currentCombination.begin(), currentCombination.end());
//				report << r << endl;
				inputCombinations.push_back(r);
			}
			if (!privateSac(currentCombination, inputCombinations, outputsCount, function)) return false;
		}
	} while (next_combination(inputsVector.begin(), inputsVector.end(), currentCombination.begin(), currentCombination.end()));
	return true;
}

// cely test...
inline int strictAvelancheCriterion(const int fromOrder, const int toOrder, const int inputsCount, const int outputsCount, const intVector & function) {
	for (int o = fromOrder; o <= toOrder; o++) {
		if (!sac(o, inputsCount, outputsCount, function)) return o - 1;
	}
	return toOrder;
}

inline int strictAvelancheCriterion(const int inputsCount, const int outputsCount, const intVector & function) {
	return strictAvelancheCriterion(0, inputsCount-2, inputsCount, outputsCount, function);
}

// ismaxOrderSAC() ?

/**
 * BENT
 */
inline float bentScore(const int inputsCount, const int outputsCount, const intVector & function)
{
    register int score = 0;
    const int inputsCombinations = 1<<inputsCount;
	// kongruence * a +
    for (int w = 0; w < inputsCombinations; w++) {
		for (int y = 0; y < outputsCount; y++) {
			register int sum = 0;

			for (int x = 0; x < inputsCombinations; x++) {
				sum += (is_odd( binaryDot(x,w) + ((function[x] & 1<<y) > 0) )) ? -1 : 1; // (-1)^(g(x) + x*w)
			}
			//sum >>= (inputsCount>>1);// sum / sqrt(2^n) = sum / 2^(n/2)
			if (fabs( ((float) sum) / (1<<(inputsCount>>1)) ) == 1.0) score++;
		}
	}
    // bent boolean funkce by mela splnovat rad 0 SACu
    if (outputsCount == 1) assert(score < inputsCombinations || sac(0, inputsCount, outputsCount, function));
    return score;
}

/**
 * Nonlinearity ...
 */
inline double computeLPMax(const intVector & function) {
	int result = 0;
	const int size = function.size();
	for (int my=1; my < size; my++) {
		for (int mx=0; mx < size; mx++) {
			int count = 0;
			for (int x=0; x < size; x++) {
				const int y = function[x];
				const int nx = hammingWeight(x&mx);
				const int ny = hammingWeight(y&my);
				const int n = (nx^ny) & 1;
				if (n==0) count++;
			}
			const int r = 2*count-size;
			const int n = r*r;
			result = std::max(n, result);
		}
	}
	return double(result) / (size * size);
}

/**
 * Diferenciality ...
 */
inline double computeDPMax(const intVector & function) {
	int result = 0;
	const int size = function.size();
	int stats[size][size];
	memset(stats, 0, size*size*sizeof(int));

	for (int dx=1; dx < size; dx++) {
		for (int x0=0; x0 < size; x0++) {
			const int x1 = x0 ^ dx;
			const int y0 = function[x0];
			const int y1 = function[x1];
			const int dy = y0 ^ y1;
			const int n = ++stats[dx][dy];
			result = std::max(n, result);
		}
	}
	return ((double) result) / size;
}

/**
 * bijektivita:
 */
inline bool isBijective(const intVector & function) {
	const int length = function.size();
	int inv[length];
	for (int x=0; x<length; x++) inv[x] = -1;
	for (int x=0; x<length; x++) {
		const int y = function[x];
		if (inv[y]!=-1) return false;
		inv[y] = x;
	}
	return true;
}

inline bool isBidirectional(const intVector & function) {
	for (int x = 0; x < int(function.size()); x++) {
		if (function[function[x]] != x) return false;
	}
	return true;
}

inline float bijectiveScore(const intVector & function) {
	float score = 0.0;
	const int halfSize = int(function.size()>>1);
	for (uint a = 0; a < function.size(); a++) {
		int sum = 0;
		for (uint x = 0; x < function.size(); x++) {
			sum += binaryDot(a, function[x]);
		}
		score += 1 - float(abs(sum-halfSize))/halfSize;
	}
	return score;
}

/**
 * branching factor :
 */
inline int computeBranchingFactor(const intVector & function) {
	int result = INT_MAX;
	const int length = function.size();
	for (int dx=1; dx<length; dx++) {
		for (int x0=0; x0<length; x0++) {
			const int x1 = x0 ^ dx;
			const int y0 = function[x0];
			const int y1 = function[x1];
			const int dy = y0 ^ y1;
			const int nx = hammingWeight(dx);
			const int ny = hammingWeight(dy);
			const int weight = nx+ny;
			result = std::min(result, weight);
		}
	}
	return result;
}

inline intVector computePolynomial(const intVector & fun) {
	intVector result(fun);
	for (uint i = 0; i < result.size(); i++) {
		const int value = result[i];
		for (uint j= i+1; j < result.size(); j++) {
			if ((j & i) == i) result[j] ^= value;
		}
	}
	return result;
}

inline int highestOneBit(int i) {
	i |= (i >>  1);
	i |= (i >>  2);
	i |= (i >>  4);
	i |= (i >>  8);
	i |= (i >> 16);
	return i - (i >> 1);
}

inline int highestBitPosition(const intVector & polynomial) {
	int mask = 0;
	for (uint i = 0; i < polynomial.size(); i++) mask |= polynomial[i];
	return highestOneBit(mask);
}

inline std::string polynomialToString(const intVector & polynomial) {
	const int highestBitPosition = ::highestBitPosition(polynomial);
	std::ostringstream result;
	for (int n = 0; n < highestBitPosition; n++) {
		const int mask = 1<<n;
		std::ostringstream sb;
		for (uint i = 0; i < polynomial.size(); i++) {
			if ((polynomial[i]&mask) == 0) continue;
			if (i==0) {
				sb << '1';
			} else {
				bool first = true;
				for (int j = 0; j < 32; j++) {
					if ((i & (1<<j)) == 0) continue;
					if (!first) sb << '*';
					first = false;
					sb << ((char) (('a' + j)));
				}
			}
			sb << " + ";
		}

		std::string str = sb.str();
		if (str.size()!=0) {
			result << str.substr(0, str.size()-3) << " , ";
		}
	}
	return result.str();
}

inline intVector polynomialDegrees(const intVector & polynomial) {
	const int highestBitPosition = ::highestBitPosition(polynomial);
	intVector result;
	for (int n = 0; n < highestBitPosition; n++) {
		const int mask = 1<<n;
		uint degree = 0;
		for (uint i = 0; i < polynomial.size(); i++) {
			if ((polynomial[i]&mask) == 0) continue;
			degree = std::max(degree, hammingWeight(i));
		}
		result.push_back(degree);
	}
	return result;
}

//class GaloisField {
//private:
//	const uint bitsCount;
//	const uint mask;
//	const int polynomial;
//	const int generator;
//	intVector logValues;
//	intVector exp;
//
//	GaloisField(int b, int p, int g) : bitsCount(b), mask((1<<bitsCount) - 1), polynomial(p), generator(g) {
//		assert(bitsCount <= 16);
//		logValues.assign(mask+1, 0);
//		exp.assign(mask, 0);
//		for (uint i=0, a=1; i<mask; i++, a = privateMul(a, generator)) {
//			if (a==0) throw std::runtime_error("wrong galois");
//			if (a==1 && i!=0) throw std::runtime_error("wrong galois");
//			if (logValues[a]!=0) throw std::runtime_error("wrong galois - double init log");
//			if (exp[i]!=0) throw std::runtime_error("wrong galois - double init exp");
//			logValues[a] = i;
//			exp[i] = a;
//		}
//		assert(isOk());
//	}
//
//	inline bool isOk() {
//		assert(logValues[1]==0);
//		for (uint i=0; i<mask; i++) assert(logValues[exp[i]] == i);
//		for (uint i=1; i<=mask; i++) assert(exp[logValues[i]] == i);
//		return true;
//	}
//
//	int mod(int a, int b) {
//        if (b<0) {
//            a = -a; b = -b;
//        }
//        int result = a%b;
//        if (result<0) result += b;
//        assert(result>=0);
//        return result;
//	}
//
//	inline int privateMul(int a, int b) const {
//		return privateMul(a, b, polynomial);
//	}
//
//	int privateMul(int a, int b, int polynomial) const {
//		int result = 0;
//		int m = mask & ~(mask>>1);
//		while (b!=0) {
//			if ((b&1) != 0) result ^= a;
//			const bool carry = (a&m) != 0;
//			a <<= 1;
//			a &= mask;
//			if (carry) a ^= polynomial;
//			b = uint(b) >> 1; // pozor na znamenko
//		}
//		return result & mask;
//	}
//
//public:
//	static GaloisField access(uint bitsCount) {
//		const int mask = 1<<bitsCount;
//		for (int g=1; g<mask; g++) {
//			if (g==1 && bitsCount!=1) continue;
//			for (int p=1; p<mask; p+=2) {
//				try {
//					return GaloisField(bitsCount, p, g);
//				} catch (std::runtime_error e) {
////					report << B << " " << p << " " << g << endl;
////					report << e.what() << endl;
//					continue;
//				}
//			}
//		}
//		throw std::runtime_error("wrong galois, GF() not found");
//	}
//
//	int div(int a, int b) const {
//		b &= mask;
//		if (b==0) return mask;
//		a &= mask;
//		if (a==0) return 0;
//		int x = (log(a) - log(b));
//		return exp[x>=0 ? x : x+mask] & mask;
//	}
//
//	int inv(int a) const {
//		a &= mask;
//		if (a==0) return mask;
//		int x = -log(a);
//		return exp[x+mask] & mask;
//	}
//
//	int log(int a) const {
//		a &= mask;
//		if (a==0) throw std::runtime_error("log(0)");
//		return logValues[a] & mask;
//	}
//
//	int mul(int a, int b) const {
//		a &= mask;
//		if (a==0) return 0;
//		b &= mask;
//		if (b==0) return 0;
//		uint x = log(a) + log(b);
//		int result = exp[x<mask ? x : x-mask] & mask;
//		assert(result == privateMul(a, b));
//		return result;
//	}
//
//	int pow(int a, int n) {
//		a &= mask;
//		if (a<=1) return a;
//		long x = (long) log(a) * n;
//		return exp[mod(x, mask)] & mask;
//	}
//
//	inline int gen() const {
//		return generator;
//	}
//};

inline int lagrangePolynomialDegree(const intVector & function, const int outputsCount) {
	assert(false);
//	const uint size = function.size(); assert(size == 1<<outputsCount);
//	if (*std::max_element(function.begin(), function.end()) != size-1) {
//		return 0;
//	}
//	const GaloisField & gf = GaloisField::access(outputsCount);
//	//	F = [[points[i][1]] for i in xrange(n)]
//	std::vector<intVector> F;
//	for (uint i = 0; i < size; i++) {
//		F.push_back(std::vector<int>(function[i]));
//	}
//
//	//	for i in xrange(1, n):
//	for (uint i = 1; i < size; i++) {
//	//	   for j in xrange(1, i+1):
//		for (uint j = 1; j < i+1; j++) {
//			//numer = F[i][j-1] - F[i-1][j-1]
//			const int numer = F[i][j-1] ^ F[i-1][j-1];
//			//denom = points[i][0] - points[i-j][0]
//			const int denom = function[i] ^ function[i - j];
//			//F[i].append(numer / denom)
//			F[i].push_back(gf.div(numer, denom));
//			//F[i].push_back(1);
//		}
//	}
//	const int var = gf.gen();
//	intVector P = F[size-1];
////  for i in xrange(n-2, -1, -1):
//	for (uint i = size-2; i > -1; i--) {
//		for (uint j = 0; j < P.size(); j++) {
//			P[j] = gf.mul(P[j], (var ^ function[i]));
//			P[j] ^= F[i][j];
//		}
//	}
//
//	report << "next: " << P.size() << endl;
//	for (uint j = 0; j < P.size(); j++) {
//		report << P[j] << endl;
//	}
//	report << "---" << endl;
//	return P.size() - std::count(P.begin(), P.end(), 0);
}

#endif /* CRITERIONS_H_ */
