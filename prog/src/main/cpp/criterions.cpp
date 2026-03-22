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
#include "criterions.h"
#include "boxes.h"

inline float computeCriterionsFunction(const CriterionsFitness fit, GAGenome &g) {
	return dynamic_cast<Sbox &>(g).computeCriterionsFunction(fit);
}

float bijectiveObjective(GAGenome &g) {
	return computeCriterionsFunction(BIJECTIVE_SCORE, g);
}

float bentAndMosacObjective(GAGenome & g) {
	return computeCriterionsFunction(BENT_AND_MOSAC, g);
}

float lpMaxFunctionObjective(GAGenome & g) {
	return computeCriterionsFunction(LP_MAX, g);
}

float dpMaxFunctionObjective(GAGenome & g) {
	return computeCriterionsFunction(DP_MAX, g);
}

float sacObjective(GAGenome &g) {
	return computeCriterionsFunction(SAC, g);
}

float branchingFactorObjective(GAGenome &g) {
	return computeCriterionsFunction(BF, g);
}

float noneCriterionObjective(GAGenome &g) {
	return computeCriterionsFunction(NONE_CRITERION, g);
}

float minimalRealizationObjective(GAGenome &g) {
	return computeCriterionsFunction(MINIMAL_REALISATION, g);
}

float polynomialDegreeObjective(GAGenome &g) {
	return computeCriterionsFunction(POLYNOMIAL_DEGREE, g);
}

float symbolicalRegresionObjective(GAGenome &g) {
	return dynamic_cast<Sbox &>(g).computeSymbolicalRegresionError();
}

float lagrangeObjective(GAGenome &g) {
	return computeCriterionsFunction(LAGRANGE, g);
}

GAGenome::Evaluator criterionsEnumToFunction(CriterionsFitness fitness) {
	switch (fitness) {
		case BENT_AND_MOSAC:
			return bentAndMosacObjective;
		case LP_MAX:
			return lpMaxFunctionObjective;
		case DP_MAX:
			return dpMaxFunctionObjective;
		case BIJECTIVE_SCORE:
			return bijectiveObjective;
		case SAC:
			return sacObjective;
		case BF:
			return branchingFactorObjective;
		case POLYNOMIAL_DEGREE:
			return polynomialDegreeObjective;
		case MINIMAL_REALISATION:
			return minimalRealizationObjective;
		case LAGRANGE:
			return lagrangeObjective;
		case NONE_CRITERION:
			return noneCriterionObjective;
		default:
			throw std::runtime_error("Unknown criterion fitness value");
	}
	return NULL;
}
