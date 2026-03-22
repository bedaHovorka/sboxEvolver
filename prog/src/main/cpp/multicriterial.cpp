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
#include "multicriterial.h"

void VegaAlgorithm::initialize(uint seed) {
	commonPreInit(seed);

	createSegments();
	mainPopulationToSegments();
	segmentsToMainPopulation();

	commonPostInit(pop);
}

inline void VegaAlgorithm::segmentsToMainPopulation() {
	pop->size(0);
	for (GAPopulationVector::const_iterator i = populationSegments.begin(); i != populationSegments.end(); i++) {
		const GAPopulation &segment = *i;
		for (int j = 0; j < segment.size(); j++) {
			GAGenome &genome = segment.individual(j);
			dynamic_cast<Sbox &>(genome).backupOrRestoreScore();
		}
		segment.scale(gaTrue);
		for (int j = 0; j < segment.size(); j++) {
			GAGenome &genome = segment.individual(j);
			genome.score(genome.score() / (segment.sum()+1));
			pop->add(genome);
		}
	}
}

inline void VegaAlgorithm::mainPopulationToSegments() {
	const uint segmentSize = populationSize() / getCriterionsSet().size();
	uint ind = 0;

	CriterionsSet::const_iterator c = getCriterionsSet().begin();
	for (GAPopulationVector::iterator i = populationSegments.begin();i != populationSegments.end(); i++, c++) {
		GAPopulation &segment = *i;
		segment.size(0);
		GAGenome::Evaluator evaluator = criterionsEnumToFunction(*c);
		for (uint j = ind; j < ind+segmentSize; j++) {
			GAGenome &genome = pop->individual(j);
			if (genome.evaluator() != evaluator) {
				genome.evaluator(evaluator);
				invalidate(&genome);
			}
			segment.add(genome);
		}
		ind += segmentSize;
		segment.evaluate(gaTrue);
	}
}

inline void VegaAlgorithm::createSegments() {
	for (CriterionsSet::const_iterator i = getCriterionsSet().begin(); i != getCriterionsSet().end(); i++) {
		GAPopulation segment;
		populationSegments.push_back(segment);
	}
}

void VegaAlgorithm::step() {
	std::swap(oldPop, pop);
	variation(oldPop);
	mainPopulationToSegments();
	segmentsToMainPopulation();
	stats.update(*pop);
}

void SpeaAlgorithm::initialize(uint seed) {
	commonPreInit(seed);
	elite.reset(new GAPopulation);
	evaluation();
	commonPostInit(populationAndElite.get());
}

void SpeaAlgorithm::step() {
	variation(populationAndElite.get());
	evaluation();
	stats.update(*populationAndElite);
}

inline bool MulticriterialGeneticAlgorithm::is1dominatingOver2(GAGenome &g1, GAGenome &g2) {
	//silna dominance
	Sbox &box1 = dynamic_cast<Sbox&>(g1);
	Sbox &box2 = dynamic_cast<Sbox&>(g2);
	const MultievaluationValues &values1 = box1.multievaluate(criterions);
	const MultievaluationValues &values2 = box2.multievaluate(criterions);
	check(values1.size() == values2.size(), "Multievaluation value sizes mismatch");
	bool isOneGreater = false;
	for (MultievaluationValues::const_iterator i1 = values1.begin() , i2 = values2.begin(); i1 != values1.end() && i2 != values2.end(); i1++, i2++) {
		check(i1->first == i2->first, "Multievaluation keys mismatch");
		double cmp = i1->second - i2->second;
		if (cmp < 0) return false;
		if (cmp > 0) isOneGreater = true;
	}
	return isOneGreater;
}

inline void MulticriterialGeneticAlgorithm::computeNondominancePopulation(const GAPopulation &pop, GAPopulation &nondominance) {
	check(pop.size() > 0, "Population must not be empty for nondominance computation");
	for (int i = 0; i < pop.size(); i++) {
		GAGenome &g1 = pop.individual(i);
		for (int j = 0; j <= nondominance.size(); j++) {
			if (j == nondominance.size()) {
				//na konci testu se vsemi z nondominance, g1 by melo patrit mezi ne (vzajemne s nimy nedominuje)
				nondominance.add(g1);
				break;
			} else {
				GAGenome &g2 = nondominance.individual(j);
				if (is1dominatingOver2(g2, g1) || g1 == g2) {
					break;// !continue vnejsiho cyklu !
				} else if (is1dominatingOver2(g1, g2)) {
					nondominance.remove(j); j--;
				}
			}
		}
		// tady nesmi byt nic, lepsi nez goto...
	}
}

// Euclidan distance
inline double SpeaAlgorithm::distance(const MultievaluationValues &values1, const MultievaluationValues &values2) {
	double sum = 0.0;
	for (MultievaluationValues::const_iterator i1 = values1.begin() , i2 = values2.begin(); i1 != values1.end() && i2 != values2.end(); i1++, i2++) {
		check(i1->first == i2->first, "Distance calculation key mismatch");
		double diff = i1->second - i2->second;
		sum += diff*diff;
	}
	return sqrt(sum);
}

inline double SpeaAlgorithm::clusterDistance(const Cluster &cluster1, const Cluster &cluster2) {
	double sum = 0.0;
	for (Cluster::const_iterator i = cluster1.begin(); i != cluster1.end(); i++) {
		for (Cluster::const_iterator j = cluster2.begin(); j != cluster2.end(); j++) {
			sum += distance((*i)->multievaluate(getCriterionsSet()), (*j)->multievaluate(getCriterionsSet()));
		}
	}
	return sum / (cluster1.size()*cluster2.size());
}

inline void SpeaAlgorithm::addCluster(Cluster *cluster, PairsForCluster &pairsForCluster, ClusterDistances &clusterDistances) {
	ClusterPairVector clusterPairs;
	for (PairsForCluster::iterator i = pairsForCluster.begin(); i != pairsForCluster.end(); i++) {
		ClusterPair* clusterPair = new ClusterPair(cluster, i->first);
		clusterDistances.insert(std::pair<double, ClusterPair*>(clusterDistance(*cluster, *(i->first)), clusterPair));
		clusterPairs.push_back(clusterPair);
		i->second.push_back(clusterPair);
	}
	pairsForCluster[cluster] = clusterPairs;
}

inline void SpeaAlgorithm::removeCluster(Cluster *cluster, PairsForCluster &pairsForCluster) {
	ClusterPairVector &clusterPairs = pairsForCluster[cluster];
	for (ClusterPairVector::iterator i = clusterPairs.begin(); i != clusterPairs.end(); i++) {
		(*i)->invalidate();
	}
	pairsForCluster.erase(cluster);
	freeAndNULL(cluster);
}

inline void SpeaAlgorithm::clustering(std::unique_ptr<GAPopulation> &elite, const uint limit) {
	if (uint(elite->size()) < limit) return;
	PairsForCluster pairsForCluster;
	ClusterDistances clusterDistances;

	// jedinec = jeden shluk, ulozit vzajemne vzdalenosti
	for (int i = 0; i < elite->size(); i++) {
		Cluster *cluster = new Cluster(1, dynamic_cast<Sbox *>(&elite->individual(i)));
		addCluster(cluster, pairsForCluster, clusterDistances);
	}

	// dokud neni pocet zhluku mensi nez 1/4 normalni populace:
	while (pairsForCluster.size() > limit) {
		//	odstranit dva nejblizssi shluky
		ClusterDistances::iterator i = clusterDistances.begin();
		if (i == clusterDistances.end()) break;
		while (!(i->second && i->second->isValid())) {
//			freeAndNULL(i->second);
//			clusterDistances.erase(i);
			i++;
		}
		Cluster *cluster = new Cluster(*(i->second->first));
		cluster->insert(cluster->end(), i->second->second->begin(), i->second->second->end());
		removeCluster(i->second->first, pairsForCluster);
		removeCluster(i->second->second, pairsForCluster);

		freeAndNULL(i->second);
		clusterDistances.erase(clusterDistances.begin(), ++i);
		//	nahradit jednim, dopocitat vzdalenosti, zaradit
		addCluster(cluster, pairsForCluster, clusterDistances);
	}

	for (ClusterDistances::iterator i = clusterDistances.begin(); i != clusterDistances.end(); i++) freeAndNULL(i->second);

	std::unique_ptr<GAPopulation> tmp(new GAPopulation);
	for (PairsForCluster::iterator i = pairsForCluster.begin(); i != pairsForCluster.end(); i++) {
		Cluster *cluster = i->first;
		check(cluster->size() > 0, "Cluster must not be empty");
		Sbox *nearest = (*(i->first))[0];
		if (cluster->size() > 1) {
			MultievaluationValues center;
			for (CriterionsSet::iterator c = getCriterionsSet().begin(); c != getCriterionsSet().end(); c++) center[*c] = 0;

			for (Cluster::const_iterator s = cluster->begin(); s != cluster->end(); s++) {
				const MultievaluationValues &values = (*s)->multievaluate(getCriterionsSet());
				for (MultievaluationValues::const_iterator c = values.begin(); c != values.end(); c++) center[c->first] += c->second;
			}
			for (MultievaluationValues::iterator c = center.begin(); c != center.end(); c++) c->second = c->second / cluster->size();

			for (Cluster::const_iterator s = cluster->begin()+1; s != cluster->end(); s++) {
				if (distance(center, (*s)->multievaluate(getCriterionsSet())) < distance(center, nearest->multievaluate(getCriterionsSet()))) {
					nearest = *s;
				}
			}
		}
		tmp->add(*nearest);
//		pairsForCluster.erase(cluster);
		freeAndNULL(cluster);
	}
	pairsForCluster.clear();
	elite = std::move(tmp);
}

inline void SpeaAlgorithm::computePowers() {
	//vsem z normalni populace score=1
	for (int i = 0; i < pop->size(); i++) pop->individual(i).score(1);

	//ke kazdemu z elity najit seznam dominovanych, pritom count++
	// spocitat silu a tu pricist vsem v seznamu
	for (int e = 0; e < elite->size(); e++) {
		intVector dominated;
		GAGenome &prominent = elite->individual(e);

		for (int i = 0; i < pop->size(); i++) {
			GAGenome &sheep = pop->individual(i);
			if (is1dominatingOver2(prominent, sheep)) {
				dominated.push_back(i);
			}
		}

		float prominentScore = float(dominated.size()) / (pop->size() + 1);
		prominent.score(prominentScore);

		for (intVector::iterator d = dominated.begin(); d != dominated.end(); d++) {
			GAGenome &sheep = pop->individual(*d);
			sheep.score(sheep.score()+prominentScore);
		}
	}
}

inline void SpeaAlgorithm::evaluation() {
	populationAndElite.reset();
	computeNondominancePopulation(*pop, *elite);
	clustering(elite, pop->size()>>2);
	computePowers();

	//sloucit elitu a pop
	populationAndElite.reset(new GAPopulation(*pop));
	for (int i = 0; i < elite->size(); i++) populationAndElite->add(elite->individual(i));
}

inline void MulticriterialGeneticAlgorithm::commonPreInit(uint seed) {
	GARandomSeed(seed);
	params.set(gaNnBestGenomes, 0);
	stats.nBestGenomes(pop->individual(0), 0);
	pop->initialize();
}

inline void MulticriterialGeneticAlgorithm::commonPostInit(GAPopulation *tmpPop) {
	params.set(gaNnBestGenomes, bestGenomes);
	stats.nBestGenomes(tmpPop->individual(0), bestGenomes);
	stats.reset(*tmpPop);

	if(!scross) GAErr(GA_LOC, className(), "initialize", gaErrNoSexualMating);
}

inline void MulticriterialGeneticAlgorithm::variation(GAPopulation *tmpPop) {
	// spolecna metoda pro VEGA a SPEA, pochazi z Galibu, pridano volani invalidate...
	int i, mut, c1, c2;
	for(i = 0;i < pop->size() - 1;i += 2){
		// takes care of odd population
		GAGenome *mom = &(tmpPop->select());
		GAGenome *dad = &(tmpPop->select());
		stats.numsel += 2; // keep track of number of selections
		c1 = c2 = 0;
		if(GAFlipCoin(pCrossover())){
			stats.numcro += (*scross)(*mom, *dad, &pop->individual(i), &pop->individual(i + 1));
			invalidate(&pop->individual(i));
			invalidate(&pop->individual(i + 1));
			c1 = c2 = 1;
		} else {
			pop->individual(i).copy(*mom);
			pop->individual(i + 1).copy(*dad);
		}
		stats.nummut += (mut = pop->individual(i).mutate(pMutation()));
		if(mut > 0) {
			invalidate(&pop->individual(i));
			c1 = 1;
		}

		stats.nummut += (mut = pop->individual(i + 1).mutate(pMutation()));
		if(mut > 0) {
			invalidate(&pop->individual(i + 1));
			c2 = 1;
		}
		stats.numeval += c1 + c2;
	}
	if(is_odd(pop->size())){
		// do the remaining population member
		GAGenome *mom = &(tmpPop->select());
		GAGenome *dad = &(tmpPop->select());
		stats.numsel += 2; // keep track of number of selections
		c1 = 0;
		if(GAFlipCoin(pCrossover())){
			stats.numcro += (*scross)(*mom, *dad, &pop->individual(i), (GAGenome*)(0));
			invalidate(&pop->individual(i));
			c1 = 1;
		} else {
			if(GARandomBit())
				pop->individual(i).copy(*mom);
			else
				pop->individual(i).copy(*dad);
		}
		stats.nummut += (mut = pop->individual(i).mutate(pMutation()));
		if(mut > 0) {
			invalidate(&pop->individual(i));
			c1 = 1;
		}
		stats.numeval += c1;
	}
	stats.numrep += pop->size();
}
