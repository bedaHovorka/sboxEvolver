/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * eda.cpp
 *
 *  Created on: Feb 21, 2010
 *      Author: Bedrich Hovorka
 *
 * Implementation of EDA algorithm
 */
#include "eda.h"

void EstimationOfDistributionAlgorithm::step() { 
  stats.nummut += variation();
  
  for(int i = 0; i < tmpPop->size(); i++)
    pop->add(tmpPop->individual(i));

  pop->evaluate();
  pop->scale();

  for(int i = 0; i < tmpPop->size(); i++)
    pop->destroy(GAPopulation::WORST, GAPopulation::SCALED);

  stats.update(*pop);
}

int EstimationOfDistributionAlgorithm::variation() {
	model->learnStructure(pop, tmpPop->size());
	return model->sampleModel(tmpPop);
}

void BmdaModel::learnStructure(GAPopulation* pop, int size) {
	//inicializace vsech kontingencnich tabulek
	memset(onesCounts, 0, length * sizeof(int));
	std::vector<BinaryCT *> tables[length];
	
	//prochazeni vsemi chromozomy:
	for (int n = 0; n < size; n++) {
		GA1DBinaryStringGenome & genome = (GA1DBinaryStringGenome &) pop->individual(n);
		// vsechny dvojice

		for (unsigned int i = 0; i < length; i++) {
			short iValue = genome.gene(i);
			onesCounts[i] += iValue;
			if (n == 0) {
				tables[i].assign(length, 0);
			}

			for (unsigned int j = i+1; j < length; j++) {
				if (tables[i][j] == 0) tables[i][j] = new BinaryCT(i, j);
				tables[i][j]->record(iValue, genome.gene(j));
			}
		}
//		cout << genome << " ";
	}
//	cout << endl;

	//je treba seradit podle velikosti zavislosti (ti co jsou pod 95% se nezaradi...)
	//pri prochazeni kontingencni tabulkou, statisticky test a zarazeni do setu
	std::set<BinaryCT, std::greater<BinaryCT > > dependencies;

	for (unsigned int i = 0; i < length; i++) {
		for (unsigned int j = i+1; j < length; j++) {
			BinaryCT *table = tables[i][j];
			if (table->chiTest(onesCounts, popSize)) {
				dependencies.insert(*table);
			}
//			cout << '[' << i << ',' << j << ']' << endl << tables[i][j] << endl;
		}
	}
	
	for (unsigned int i = 0; i < length; i++) {
		for (unsigned int j = i+1; j < length; j++) {
			delete tables[i][j];
		}
	}

	rootsClear();
//	report << "dep: " << dependencies.size() << endl;
	if (!dependencies.size()) return;// UMDA
	
	//inicializace pripravenych uzlu
	intVector readyNodes;
	BmdaNode *nodes[length];

	for (unsigned int i = 0; i < length; i++) {
		readyNodes.push_back(i);
		nodes[i] = new BmdaNode(i);
	}

	while (readyNodes.size() > 0) {
		//dalsi korenovy uzel
		intVector::iterator nextNodeNumber = readyNodes.begin()+GARandomInt(0, readyNodes.size()-1);
		roots.push_back(nodes[*nextNodeNumber]);
		readyNodes.erase(nextNodeNumber);
		if (readyNodes.size() == 0) break;

		// projit ready nodes
		std::vector<std::set<BinaryCT>::iterator> toErase;
		for (std::set<BinaryCT>::iterator i = dependencies.begin(); i != dependencies.end(); i++) {
			// paruje se uz zpracovany s jeste nezpracovanym
			bool i_isReady = binary_search(readyNodes.begin(), readyNodes.end(), i->getIPosition());
			bool j_isReady = binary_search(readyNodes.begin(), readyNodes.end(), i->getJPosition());
			if (i_isReady == j_isReady) continue;

			int dependentNodeNumber = i_isReady ? i->getIPosition() : i->getJPosition();
			int parentNodeNumber = i_isReady ? i->getJPosition() : i->getIPosition();
			readyNodes.erase(find(readyNodes.begin(), readyNodes.end(), dependentNodeNumber));
			BmdaNode *dependentNode = nodes[dependentNodeNumber];
			dependentNode->setCountWithOne(i->get(1,1)); // pocet dvojic (1,1) vzdy v vpravem dolnim rohu tabulky
			dependentNode->setCountWithZero(i_isReady ? i->get(1,0) : i->get(0,1));// dvojice (0,1) zalezi na poradi i,j

			dependentNode->setParent(nodes[parentNodeNumber]);
			nodes[parentNodeNumber]->addChild(dependentNode);
			toErase.push_back(i);
		}
		for (std::vector<std::set<BinaryCT>::iterator>::iterator i = toErase.begin(); i != toErase.end(); i++) {
			dependencies.erase(*i);
		}
	}
}

int BmdaModel::sampleModel(GAPopulation* tmpPop) {
//	report << "roots: " << roots.size() << endl;
	if (roots.size() == length || roots.size() == 0) return EdaModel::sampleModel(tmpPop);// UMDA
	
	for (int i=0; i < tmpPop->size(); i++) {
		GA1DBinaryStringGenome *genome = (GA1DBinaryStringGenome *) &tmpPop->individual(i);
		BmdaNodeVector work(roots);
		
		while (work.size() > 0) {
			BmdaNode* current = work.front();
			work.insert(work.end(), current->childrenBegin(), current->childrenEnd());
			work.erase(work.begin());
			int position = current->getPosition();
			
			if (current->getParent()) {
				int parentPosition = current->getParent()->getPosition();
				bool parentValue = genome->gene(parentPosition);
				
				// podminena pravdepodobnost P(current=1 | parent=x) = P(current=1 ^ previous=x) / P (previous=x)
				float currentValue = (parentValue) ? ((float) current->getCountWithOne()) / onesCounts[parentPosition] :
						(onesCounts[parentPosition] == popSize) ? 0 : 
							((float) current->getCountWithZero()) / (popSize - onesCounts[parentPosition]);
				assert(currentValue >= 0 && currentValue <= 1);
				genome->gene(position, GARandomFloat(0, 1) < currentValue);
			} else {
				genome->gene(position, GARandomInt(0, popSize) <= onesCounts[position]);
			}
		}
	}
	
	return tmpPop->size();
}

void EdaModel::learnStructure(GAPopulation* pop, int size) {//UMDA
	// spocte pocet jednicek na jednotlivych pozicich
	memset(onesCounts, 0, length * sizeof(int));
	
	for (int n = 0; n < size; n++) {
		GA1DBinaryStringGenome & genome = (GA1DBinaryStringGenome &) pop->individual(n);
		for (unsigned int i = 0; i < length; i++) {
			onesCounts[i] += genome.gene(i);
		}
	}
	
	for (unsigned int i = 0; i < length; i++) {
		assert(onesCounts[i] <=pop->size() && onesCounts[i] >= 0);
	}
}

int EdaModel::sampleModel(GAPopulation* tmpPop) {//UMDA
//	cout << "tmpPop size " << tmpPop->size() << endl; 
//	cout << "new genomes: ";
	
	for (int n=0; n < tmpPop->size(); n++) {
		GA1DBinaryStringGenome *child = (GA1DBinaryStringGenome *) &tmpPop->individual(n);
		for (int i = 0; i < child->length(); i++) {
			child->gene(i, GARandomInt(0, popSize) <= onesCounts[i]);
		}
//		cout << *child << " ";
	}
//	cout << endl;
	return tmpPop->size();
}

BmdaNode::~BmdaNode() {
	deleteMembers(children.begin(), children.end());
}
