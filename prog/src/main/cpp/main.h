/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * main.h
 *
 *  Created on: Mar 11, 2010
 *      Author: Bedrich Hovorka
 *
 * Interface to python, declaration of criterions combinations
 */
#ifndef MAIN_H_
#define MAIN_H_

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <string>
#include <functional>
#include <cstdarg>
#include <stdexcept>

#include "common.h"
#include "eda.h"
#include "cgp.h"
#include "softwareSbox.h"
#include "criterions.h"
#include "multicriterial.h"


extern "C" { // je treba ceckova deklarace...
	typedef struct {
		SboxSearchAlgorithmBase *algorithm;
//		GAGenome *progenitor;
	} TSearching;

	typedef struct {
		GAGenome *ptr;
	} TGenome;

	typedef struct {
		float online; //average of all scores
		float offlineMax; // average of the maximum scores
		float offlineMin; // average of the minimum scores
		float maxEver; // maximum score ever encountered
		float minEver; // minimum score ever encountered

		int generation;  // current generation number
		float convergence; // current convergence. Here convergence means the ratio of the nth previous best-of-generation to the current best-of-generation.
		int selections; // number of selections that have occurred since initialization
		int crossovers; // number of crossovers that have occurred since initialization
		int mutations; // number of mutations that have occurred since initialization
		int replacements; // number of replacements that have occurred since initialization
		int nBestGenomes;
	} TStatistics;

	typedef enum {
		BINARY,
		PERMUTATION,
		CGP,
		SOFTWARE_IMPL
	} GenomeType;

	// criterionsfittness nyni v criterions

	typedef enum {
		GENERATION,
		CONVERGENCE,
		POPCONVERGENCE
	} TerminatorCondition;

	void processSearching(TSearching, TerminatorCondition, uint seed);
	void closeSearching(TSearching);
	const PyAPI_FUNC(PyObject *) simpleReportSearching(TSearching searching);
	const PyAPI_FUNC(PyObject *) bestPopulationStringsSearching(TSearching);
	void setMiniMaxiSearching(TSearching, bool minimize);
	TStatistics getStatisticsSearching(TSearching searching, float *bestPopulationScores, float *bestPopulationCriterionsValues, int *bestPopulationOutputs);

	void parallelProcessSearching(const int taskCount, const TSearching tasks[], TerminatorCondition condition, const uint seeds[]);

	TSearching newEdaSearching(TGenome g, int popsize, int ngen, bool bdma);
	TSearching newParallelRandomSearching(TGenome g, int popsize, int ngen, float pMut);
	TSearching newGaSearching(TGenome g, int popsize, int ngen, float pMut, float pCross, bool elitism);
	TSearching newRandomSearching(TGenome g, int bestGenomes, int ngen);
	TSearching newHeuristicSearching(TGenome g, uint maxStates, uint ngen);
	TSearching newVegaSearching(TGenome g, int popsize, int ngen, float pMut, float pCross, const int criterionsCount, const CriterionsFitness *criterions);
	TSearching newSpeaSearching(TGenome g, int popsize, int ngen, float pMut, float pCross, const int criterionsCount, const CriterionsFitness *criterions);

	TGenome createGenome(GenomeType type, CriterionsFitness function, int argc, ...);
	TGenome createSymbolicalRegresionGenome(GenomeType type, int *expectedOutputs, int argc, ...);

	GAGenome *oneMaxGenome(int chromozomeLength);

	void testSoftwareBox();
	void initialization();
}

class RandomSearchAlgorithm : public SboxSearchAlgorithmBase {
public:
	GADefineIdentity("RandomSearchAlgorithm", 520);
	RandomSearchAlgorithm(const GAGenome &g) : SboxSearchAlgorithmBase(g) {}
	virtual ~RandomSearchAlgorithm() {}

	virtual void step();
	RandomSearchAlgorithm & operator++() { step(); return *this; }
};

class ParallelRandomSearchAlgorithm : public SboxSearchAlgorithmBase {
private:
	GAGenome *oldBestGenome;
public:
	GADefineIdentity("ParallelRandomSearchAlgorithm", 288);
	explicit ParallelRandomSearchAlgorithm(const GAGenome & genome) : SboxSearchAlgorithmBase(genome), oldBestGenome(NULL) {}

	virtual void step();
	ParallelRandomSearchAlgorithm & operator++() { step(); return *this; }
	virtual ~ParallelRandomSearchAlgorithm() {
		freeAndNULL(oldBestGenome);
	}
};

class HeuristicSearchAlgorithm : public SboxSearchAlgorithmBase {
private:
	const uint maxStates;
public:
	GADefineIdentity("RandomSearchAlgorithm", 520);
	HeuristicSearchAlgorithm(const GAGenome &g, uint ms) : SboxSearchAlgorithmBase(g), maxStates(ms) {}
	virtual ~HeuristicSearchAlgorithm() {}

	virtual void evolve(unsigned int seed=0);
};

inline void stopOnError(std::string message);

inline void check(bool x, std::string message) {
	if (!x) {
		stopOnError(message);
	}
};

#endif /* MAIN_H_ */
