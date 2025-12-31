/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * multicriterial.h
 *
 *  Created on: Mar 30, 2010
 *      Author: Bedrich Hovorka
 */

#ifndef MULTICRITERIAL_H_
#define MULTICRITERIAL_H_

#include "common.h"
#include "criterions.h"
#include "boxes.h"

class MulticriterialGeneticAlgorithm : public SboxSearchAlgorithmBase {
private:
	const CriterionsSet criterions;
	uint bestGenomes;
public:
	MulticriterialGeneticAlgorithm(const GAGenome &progenitor, const CriterionsSet cv) : SboxSearchAlgorithmBase(progenitor), criterions(cv), bestGenomes(0) {}

	explicit MulticriterialGeneticAlgorithm(const MulticriterialGeneticAlgorithm & orig) : SboxSearchAlgorithmBase(orig) {}
	virtual ~MulticriterialGeneticAlgorithm() {}

protected:
	inline const CriterionsSet &getCriterionsSet() const {
		return criterions;
	}

	virtual const GAPopulation& population(const GAPopulation&) {
		assert(false);
	}

	inline uint getBestGenomes() {
		return bestGenomes;
	}

	virtual int nBestGenomes(unsigned int n) {
		return bestGenomes = n;
	}

	inline void variation(GAPopulation *tmpPop);
	inline void commonPreInit(uint seed);
	inline void commonPostInit(GAPopulation *tmpPop);
	inline bool is1dominatingOver2(GAGenome &g1, GAGenome &g2);
	inline void computeNondominancePopulation(const GAPopulation &pop, GAPopulation &nondominance);

	inline void invalidate(GAGenome *genome) {
		dynamic_cast<Sbox *>(genome)->invalidate();
	}
};

class VegaAlgorithm : public MulticriterialGeneticAlgorithm {
private:
	typedef std::vector<GAPopulation> GAPopulationVector;
	GAPopulationVector populationSegments;
	GAPopulation *oldPop;
public:
	GADefineIdentity("VegaAlgorithm", 305);
	VegaAlgorithm(const GAGenome &progenitor, const CriterionsSet cv) : MulticriterialGeneticAlgorithm(progenitor, cv), oldPop(NULL) {
		populationSegments.reserve(cv.size());
		oldPop = pop->clone();
	}
	explicit VegaAlgorithm(const VegaAlgorithm & orig) : MulticriterialGeneticAlgorithm(orig), populationSegments(orig.populationSegments) {}
	virtual ~VegaAlgorithm() {
		freeAndNULL(oldPop);
	}
	VegaAlgorithm & operator++() { step(); return *this; }

	virtual int populationSize(unsigned int n) {
		GAGeneticAlgorithm::populationSize(n);
		oldPop->size(n);
		return n;
	}

	virtual int populationSize() {
		return GAGeneticAlgorithm::populationSize();
	}
protected:
	virtual void step();
	virtual void initialize(unsigned int seed=0);
private:
	inline void segmentsToMainPopulation();
	inline void mainPopulationToSegments();
	inline void createSegments();
};

class SpeaAlgorithm : public MulticriterialGeneticAlgorithm {
private:
	GAPopulation *elite, *populationAndElite;
public:
	GADefineIdentity("SpeaAlgorithm", 306);
	SpeaAlgorithm(const GAGenome &progenitor, const CriterionsSet cv) : MulticriterialGeneticAlgorithm(progenitor, cv), elite(NULL), populationAndElite(NULL) {
		minimaxi(GAGeneticAlgorithm::MINIMIZE);
	}
	explicit SpeaAlgorithm(const SpeaAlgorithm & orig) : MulticriterialGeneticAlgorithm(orig) {}
	virtual ~SpeaAlgorithm() {
		freeAndNULL(elite);
		freeAndNULL(populationAndElite);
	}
	SpeaAlgorithm & operator++() { step(); return *this; }
protected:
	virtual void step();
	virtual void initialize(unsigned int seed=0);
private:
	inline void evaluation();
	inline void computePowers();

	typedef std::vector<Sbox*> Cluster;

	class ClusterPair : public std::pair<Cluster*, Cluster*> {
	private:
		bool valid;
	public:
		ClusterPair(Cluster *cluster1, Cluster *cluster2) : std::pair<Cluster*, Cluster*>(std::min(cluster1, cluster2), std::max(cluster1, cluster2)), valid(true) {}

		inline bool isValid() const {
			return valid;
		}

		inline void invalidate() {
			valid = false;
		}
	};

	typedef std::vector<ClusterPair* > ClusterPairVector;
	typedef std::multimap<double, ClusterPair*> ClusterDistances;
	typedef std::map<Cluster*,  ClusterPairVector> PairsForCluster;

	inline double clusterDistance(const Cluster &cluster1, const Cluster &cluster2);
	inline void clustering(GAPopulation* &elite, const uint limit);
	inline double distance(const MultievaluationValues &values1, const MultievaluationValues &values2);
	inline void addCluster(Cluster *cluster, PairsForCluster &pairsForCluster, ClusterDistances &clusterDistances);
	inline void removeCluster(Cluster *cluster, PairsForCluster &pairsForCluster);

	inline void checkNonDominance() {
#ifndef NDEBUG
		for(int i = 0;i < elite->size();i++){
			invalidate(&elite->individual(i));
		}
		for (int i = 0; i < elite->size(); i++) {
			for (int j = 0; j < elite->size(); j++) {
				if (is1dominatingOver2(elite->individual(i), elite->individual(j))) {
					Sbox &box1 = dynamic_cast<Sbox&>(elite->individual(i));
					Sbox &box2 = dynamic_cast<Sbox&>(elite->individual(j));
					box1.multievalutateReport(getCriterionsSet());
					box2.multievalutateReport(getCriterionsSet());
					assert(false);
				}
			}
		}
#endif
	}

	inline const virtual GAPopulation & bestResults()
	{
		clustering(elite, uint(getBestGenomes()));
		checkNonDominance();
		return *elite;
	}
};

#endif /* MULTICRITERIAL_H_ */
