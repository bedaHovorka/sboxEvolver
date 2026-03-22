/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * eda.h
 *
 *  Created on: Feb 21, 2010
 *      Author: Bedrich Hovorka
 *
 * Implementation of EDA algorithm
 */
#ifndef SBOXEVOLUTION_EDA
#define SBOXEVOLUTION_EDA

#include "common.h"
#include <set>
#include <functional>
#include <vector>
#include <algorithm>
#include <ga/GA1DBinStrGenome.h> // doveze i random

const double CHI_ON_DF1_AT95 = 3.8415;

template <int N> class ContingentTable {// N=pocet alel
private:
	bool tested;
	const int pos_i, pos_j;
	float testResult;
	int data[N][N];// 0, .., N-1 - jednotlive cetnosti
public:

	ContingentTable(int ii, int jj) : tested(false), pos_i(ii), pos_j(jj), testResult(CHI_ON_DF1_AT95) {
		memset(data, 0, N*N*sizeof(int));
	};

	ContingentTable(const ContingentTable<N> &old) : tested(old.tested), pos_i(old.pos_i), pos_j(old.pos_j), testResult(old.testResult) {
		memcpy(data, old.data, N*N*sizeof(int));
	};

	virtual ~ContingentTable() {};

	ContingentTable& operator=(const ContingentTable& orig){
		throw std::runtime_error("ContingentTable assignment not supported");
		return *this;
	}

	bool operator>(const ContingentTable<N> &other) const {
		check(tested && other.tested, "Contingent tables must be tested before comparison");
		return testResult > other.testResult;
	};

	inline void record(int i, int j) {
		data[i][j]++;
	};

	inline int get(int i, int j) const {
		return data[i][j];
	}

	inline bool chiTest(const int *onesCounts, const int popSize) {
		float result = 0.0;

		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				result += ((float) (data[i][j]*data[i][j])) / ((float)(onesCounts[pos_i]*onesCounts[pos_j]));
			}
		}
		result--;
		result *= popSize;
		tested = true;
		testResult = result;
		return result > CHI_ON_DF1_AT95;// !!! funguje zatim pouze pro binaryCT (N == 2) !!!
	};

	inline int getIPosition() const {
		return pos_i;
	}

	inline int getJPosition() const {
		return pos_j;
	}

	inline float getTestResult() const {
		return testResult;
	}
};

typedef ContingentTable<2> BinaryCT;

class EdaModel {
protected:
	unsigned int length;
	int popSize;
	int *onesCounts;// apriory propabilities... UMDA
public:
	explicit EdaModel(int l, int p) : length(l), popSize(p) {
		onesCounts = new int[length];
		memset(onesCounts, 0, length * sizeof(int));
	}
	explicit EdaModel(EdaModel &old) {
		popSize = old.popSize;
		length = old.length;
		onesCounts = new int[length];
		memcpy(onesCounts, old.onesCounts, length * sizeof(int));
	}
	virtual ~EdaModel() {
		delete[] onesCounts;
	}
	virtual void learnStructure(GAPopulation*, int size);
	virtual int sampleModel(GAPopulation*);
};

class BmdaNode;
typedef std::vector<BmdaNode *> BmdaNodeVector;

class BmdaNode {
private:
	const int position;
	int countWithOne;
	int countWithZero;

	BmdaNode *parent;
	BmdaNodeVector children;
public:
	explicit BmdaNode(int pos, int o=0, int z=0, BmdaNode *par=NULL) : position(pos), countWithOne(o),
			countWithZero(z), parent(par) {}
	virtual ~BmdaNode();

	inline void addChild(BmdaNode *node) {
		children.push_back(node);
	}

	inline BmdaNodeVector::const_iterator childrenBegin() const {
		return children.begin();
	}

	inline BmdaNodeVector::const_iterator childrenEnd() const {
		return children.end();
	}

	inline void setParent(BmdaNode *node) {
		parent = node;
	}

	inline BmdaNode *getParent() const {
		return parent;
	}

	inline int getPosition() const {
		return position;
	}

	inline int getCountWithZero() const {
		return countWithZero;
	}

	inline int getCountWithOne() const {
		return countWithOne;
	}

	inline void setCountWithZero(int value) {
		countWithZero = value;
	}

	inline void setCountWithOne(int value) {
		countWithOne = value;
	}
};

class BmdaModel: public EdaModel {
private:
	BmdaNodeVector roots;
public:
	inline void rootsClear() {
		deleteMembers(roots.begin(), roots.end());
		roots.clear();
	}
	explicit BmdaModel(int l, int p) : EdaModel(l, p) {}
	virtual ~BmdaModel() {
		rootsClear();
	}
	virtual void learnStructure(GAPopulation*, int size);
	virtual int sampleModel(GAPopulation*);
};

class EstimationOfDistributionAlgorithm : public SboxSearchAlgorithmBase {
private:
	int genomeLength;
	std::unique_ptr<EdaModel> model;
	std::unique_ptr<GAPopulation> tmpPop;
public:
	GADefineIdentity("EstimationOfDistributionAlgorithm", 288);
	EstimationOfDistributionAlgorithm(const GAGenome& g, bool bmda, int popsize) :
		SboxSearchAlgorithmBase(g),
		genomeLength(((GA1DBinaryStringGenome &) g).length()),
		model(bmda ? static_cast<EdaModel*>(new BmdaModel(genomeLength, popsize)) : new EdaModel(genomeLength, popsize)) {

		populationSize(popsize);
		float pRepl = gaDefPRepl;
		float n = ((pRepl*(float)pop->size() < 1) ? 1 : pRepl*(float)pop->size());
		tmpPop.reset(new GAPopulation(pop->individual(0), (unsigned int)n));
	}
	virtual ~EstimationOfDistributionAlgorithm() {}
	void step();
	EstimationOfDistributionAlgorithm & operator++() { step(); return *this; }
protected:
	int variation();
};

template <int N> void printLine(int i, std::ostream& output, const ContingentTable<N>& table) {
	for (int j = 0; j < N; j++) {
		output << table.get(i,j) << ' ';
	}
	// cout << "| " << table.data[i][N] << endl;
}

template <int N> std::ostream& operator<<(std::ostream& output, const ContingentTable<N> & table) {
	for (int i = 0; i < N; i++) {
		printLine(i, output, table);
	}
//   output << "----------------------" << endl;
//   printLine(N, output, table);
	output << table.getTestResult() << endl;

	return output;
}

template <int N> std::ostream& operator<<(std::ostream& output, ContingentTable<N> *table) {
	if (table) output << *table;
	return output;
}

#endif
