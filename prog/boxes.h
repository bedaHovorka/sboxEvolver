/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * boxes.h
 *
 *  Created on: Apr 2, 2010
 *      Author: Bedrich Hovorka
 */

#ifndef BOXES_H_
#define BOXES_H_

#include "common.h"
#include "criterions.h"
#include <sstream>

class Sbox;

typedef std::vector<Sbox *> SboxPtrVector;

//!!! interface, dynamic
class Sbox {
private:
	MultievaluationValues multievaluationValues;
	bool multievaluated, scoreBackuped;
	float backupedScore;
	intVector *expectedValues;
public:
	virtual int output(int x) const = 0;
	virtual int inputsCount() const = 0;
	virtual int outputsCount() const = 0;
	virtual const SboxPtrVector neigboursInStateSpace() const = 0;

protected:
	virtual ~Sbox() {}
	explicit Sbox() : multievaluationValues(), multievaluated(false), scoreBackuped(false), backupedScore(0.0), expectedValues(NULL) {}
	explicit Sbox(const Sbox &orig) : multievaluationValues(orig.multievaluationValues), multievaluated(orig.multievaluated),
			scoreBackuped(orig.scoreBackuped), backupedScore(orig.backupedScore), expectedValues(orig.expectedValues) {}

public:
	operator GAGenome&() {return dynamic_cast<GAGenome&>(*this);}

//	inline void test() {
//		assert(multievaluated);
//		for (MultievaluationValues::iterator i = multievaluationValues.begin(); i != multievaluationValues.end(); i++) {
//			report << this << " " << i->first << " " << i->second << endl;
//		}
//	}

	virtual void copy(const Sbox& orig) {
		multievaluationValues = orig.multievaluationValues;
		multievaluated = orig.multievaluated;
		scoreBackuped = orig.scoreBackuped;
		backupedScore = orig.backupedScore;
	}

	virtual int realizationEfficiency() const {
		return 0; // pokud nejaky genome umi minimalizovat i vypocetni naroky, predefinuje...
	}

	inline const intVector computeOutputs() const {
		const int inputsCombinations = 1<<inputsCount();
		intVector outputs;
	    outputs.reserve(inputsCombinations);
	    for(int x = 0;x < inputsCombinations;x++){
	    	const int y = output(x);
	    	assert( (y < 1<<outputsCount()) && (y >= 0));
	    	outputs.push_back(y);
	    }
	    return outputs;
	}

	inline float computeCriterionsFunction(const CriterionsFitness fitness, const intVector &outputsArg=intVector()) {
	    const intVector &outputs = (outputsArg.empty()) ? computeOutputs() : outputsArg;

		switch (fitness) {
			case BENT_AND_MOSAC:
			{
				int inputsCount = this->inputsCount();
				int outputsCount = this->outputsCount();
				int inputsCombinations = 1<<inputsCount;
				float score = bentScore(inputsCount, outputsCount, outputs);
				float res = (score >= inputsCombinations) ? score + strictAvelancheCriterion(1, inputsCount-2, inputsCount, 1, outputs) : score;
				return res;
			}
			case LP_MAX:
				return -log2(computeLPMax(outputs));
			case DP_MAX:
				return -log2(computeDPMax(outputs));
			case BIJECTIVE_SCORE:
				return bijectiveScore(outputs);
			case SAC: {
				int inputsCount = this->inputsCount();
				int outputsCount = this->outputsCount();
				return strictAvelancheCriterion(inputsCount, outputsCount, outputs) + 1;
			}
			case BF:
				return computeBranchingFactor(outputs);
			case POLYNOMIAL_DEGREE: {
				const intVector &polynomial = computePolynomial(outputs);
				const intVector &degrees = polynomialDegrees(polynomial);
				int outputsCount = this->outputsCount();
//				std::ostringstream sb;
//				for (intVector::const_iterator i = outputs.begin(); i != outputs.end(); i++) {
//					sb << *i << ',';
//				}
//				report << sb.str() << endl;

				if (degrees.size() < uint(outputsCount)) return 0;
				return *std::min_element(degrees.begin(), degrees.begin()+outputsCount);
			}
			case MINIMAL_REALISATION:
				return realizationEfficiency();
			case LAGRANGE:
				return lagrangePolynomialDegree(outputs, outputsCount());
			case NONE_CRITERION:
				assert(false);
				report << "calling none fittness objective" << endl;
				return 0.0;
			default:
				assert(false);
				break;
		}
	    return 0.0;
	}

	inline const MultievaluationValues &multievaluate(const CriterionsSet &criterions) {
		if (multievaluated) return multievaluationValues; // tohle nebude fungovat akorat za zmeny kriterii behem programu
		const intVector &outputs = computeOutputs();
		for (CriterionsSet::iterator i = criterions.begin(); i != criterions.end(); i++) {
			multievaluationValues[*i] = computeCriterionsFunction(*i, outputs);
		}
		multievaluated = true;
		return multievaluationValues;
	}

	inline void multievalutateReport(const CriterionsSet &criterions) {
		const MultievaluationValues &values = multievaluate(criterions);
		std::ostringstream out;
		out << '{';
		for (MultievaluationValues::const_iterator c = values.begin(); c != values.end(); c++) {
			if (c != values.begin()) out << ", ";
			out << c->first << " = " << c->second;
		}
		report << out.str() << "} " << *this << endl;
	}

	inline void backupOrRestoreScore() {
		if (scoreBackuped) {
			GAGenome(*this).score(backupedScore);
		} else {
			backupedScore = GAGenome(*this).score();
			scoreBackuped = true;
		}
	}

	inline void invalidate() {
		multievaluated = false;
		scoreBackuped = false;
	}

	inline void setExpectedValues(intVector* & ev) {
		expectedValues = ev;
	}

	inline int computeSymbolicalRegresionError() {
		assert(expectedValues);
		const intVector &outputs = computeOutputs();
		assert(expectedValues->size() == outputs.size());
		int sum = 0;
		for (intVector::const_iterator o = outputs.begin(), e = expectedValues->begin(); o != outputs.end(); o++, e++) {
			const int diff = *o - *e;
			sum += diff * diff;
		}

//		report << *this << " " << sum << endl;
		return sum;
	}
};

class BinarySboxGenome : public Sbox, public GA1DBinaryStringGenome {
private:
	const int inputs;
	const int outputs;
public:
	GADefineIdentity("BooleanFunctionGenome", 207);
	BinarySboxGenome(int i, int o, Evaluator e) : Sbox(), GA1DBinaryStringGenome((1<<i)*o, e), inputs(i), outputs(o) {};
	BinarySboxGenome(const BinarySboxGenome & old) : Sbox(old), GA1DBinaryStringGenome(old), inputs(old.inputs), outputs(old.outputs) {};
	virtual ~BinarySboxGenome() {};

	BinarySboxGenome& operator=(const GAGenome& arg) {
		if(&arg != this) copy(arg);
		return *this;
	}

	virtual void copy(const GAGenome& orig) {
		GA1DBinaryStringGenome::copy(orig);
		Sbox::copy((BinarySboxGenome &) orig);
	}

	virtual GAGenome* clone(CloneMethod) const {return new BinarySboxGenome(*this);}

	virtual int output(int x) const {
		int result = 0;
		const int inputsCombination = 1 << inputsCount();
		for (int i = 0; i < outputsCount(); i++) {
			result += int(gene(x + i*inputsCombination)) << i;
		}
		return result;
	}

	virtual int inputsCount() const {
		return inputs;
	}

	virtual int outputsCount() const {
		return outputs;
	}

	int write(std::ostream & fout) const {
		const uint mask = (1<<inputsCount()) - 1;
		for (unsigned int i=0; i<nx; i++) {
			if (i && !(i&mask)) fout << " , "; //oddelovac mezi booleovskymi funkcemi
			fout << gene(i);
		}
		return 0;
	}

	inline bool isEvaluated() {
		return _evaluated;
	}

	virtual const SboxPtrVector neigboursInStateSpace() const {
		SboxPtrVector genomes;
		for (int i = 0; i < length(); i++) {
			BinarySboxGenome *genome = new BinarySboxGenome(*this);
			genome->_evaluated = gaFalse;
			genome->gene(i, !genome->gene(i));
			genomes.push_back(genome);
		}
		return genomes;
	}
};

template < typename T > class PermutationSboxGenomeTemplate : public Sbox, public GA1DArrayGenome<T> {
public:
	static void Init(GAGenome&);
	static int Mutate(GAGenome &, float);
	static int Cross(const GAGenome&, const GAGenome&, GAGenome*, GAGenome*);
private:
	const bool bijective;
	const T inputs, outputs;
public:
	GADefineIdentity("PermutationSboxGenome", 284);
	PermutationSboxGenomeTemplate(bool bij, T i, T o, GAGenome::Evaluator e) : Sbox(), GA1DArrayGenome<T>(1<<i, e, 0), bijective(bij), inputs(i), outputs(o) {
		// 2^inputs musi byt nasobkem 2^outputs <=> i >= o
		assert(i>=o);
		initializer(Init);
		mutator(Mutate);
		if (bijective) {
			assert(i == o);
			crossover(Cross);
		} else {
			crossover(GA1DArrayGenome<T>::OnePointCrossover);
		}
	}

	PermutationSboxGenomeTemplate(const PermutationSboxGenomeTemplate &orig) : Sbox(orig), GA1DArrayGenome<T>(orig), bijective(orig.bijective), inputs(orig.inputs),
			outputs(orig.outputs){}
	virtual ~PermutationSboxGenomeTemplate() {}

	PermutationSboxGenomeTemplate& operator=(const GAGenome& arg) {
		if(&arg != this) copy(arg);
		return *this;
	}

	virtual void copy(const GAGenome& arg) {
		PermutationSboxGenomeTemplate &orig = (PermutationSboxGenomeTemplate &) arg;
		GA1DArrayGenome<T>::copy(orig);
		Sbox::copy(orig);
		assert(bijective == orig.bijective);
		assert(inputs == orig.inputs);
		assert(outputs == orig.outputs);
	}

	virtual GAGenome* clone(GAGenome::CloneMethod) const {return new PermutationSboxGenomeTemplate<T>(*this);}

	virtual int output(int x) const {
		return (*this)[x];
	}

	virtual int inputsCount() const {
		return inputs;
	}

	virtual int outputsCount() const {
		return outputs;
	}

	virtual bool mustBijective() const {
		return bijective;
	}

	virtual const SboxPtrVector neigboursInStateSpace() const {
		SboxPtrVector neighbours;
		const T mask = (1<<outputs)-1;
		if (bijective) {
			// vsecna mozna prohozeni 2 genu
			for (int i = 0; i < 1<<inputs; i++) {
				for (int j = i+1; j < 1<<inputs; j++) {
					PermutationSboxGenomeTemplate<T> *copy = new PermutationSboxGenomeTemplate(*this);
					std::swap((*copy)[i], (*copy)[j]);
					copy->_evaluated = gaFalse;
					neighbours.push_back(copy);
				}
			}
		} else {
			// soused: inkrementace jednoho genu mod 2^outputs
			for (int i = 0; i < 1<<inputs; i++) {
				PermutationSboxGenomeTemplate<T> *copy = new PermutationSboxGenomeTemplate(*this);
				(*copy)[i]++;
				(*copy)[i]&=mask;
				copy->_evaluated = gaFalse;
				neighbours.push_back(copy);
			}
		}
		return neighbours;
	}

	inline void invalidate() {
		Sbox::invalidate();
		GAGenome::_evaluated = gaFalse;
	}

	inline bool isEvaluated() {
		return GAGenome::_evaluated;
	}
private:
	static T prand(int n) {
		return GARandomInt(0, n -1);
	}

	inline void randomize() {
		//2^outputs rozhazej 2^inputs krat
		const T mask = (1<<outputs)-1;
		assert(GA1DArrayGenome<T>::sz == (uint)1<<inputs);
		for (T i = 0; i < 1<<inputs; i++) {
			(*this)[i] = i&mask;
		}
		std::random_shuffle(GA1DArrayGenome<T>::a, GA1DArrayGenome<T>::a + GA1DArrayGenome<T>::sz, prand);
	}

	inline int mutate(const float pmut) {
		uint m = 0;
		if (bijective) {
			for (int i = 0; i < 1<<inputs; i++) {
				for (int j = i+1; j < 1<<inputs; j++) {
					if (GAFlipCoin(pmut)) {
						std::swap((*this)[i], (*this)[j]);
						m++;
					}
				}
			}
		} else {
			for (T *i = GA1DArrayGenome<T>::a; i < GA1DArrayGenome<T>::a+GA1DArrayGenome<T>::sz; i++) {
					if (GAFlipCoin(pmut)) {
					*i = GARandomInt(0, (1<<outputs)-1);
					m++;
				}
			}
		}
		if (m > 0) GAGenome::_evaluated = gaFalse;
		return m;
	}
};

template <typename T> void PermutationSboxGenomeTemplate<T>::Init(GAGenome& g) {
	PermutationSboxGenomeTemplate<T> &genome = (PermutationSboxGenomeTemplate<T> &) g;
	genome.randomize();
}

template <typename T>  int PermutationSboxGenomeTemplate<T>::Mutate(GAGenome &g, float pmut) {
	PermutationSboxGenomeTemplate<T> &genome = (PermutationSboxGenomeTemplate<T> &) g;
	return genome.mutate(pmut);
}

template <typename T > int PermutationSboxGenomeTemplate<T>::Cross(const GAGenome&p1, const GAGenome&p2, GAGenome*o1, GAGenome*o2) {
	int res = GA1DArrayGenome<T>::OrderCrossover(p1, p2, o1, o2);
	dynamic_cast<PermutationSboxGenomeTemplate<T> *>(o1)->invalidate();
	dynamic_cast<PermutationSboxGenomeTemplate<T> *>(o2)->invalidate();
	return res;
}

typedef PermutationSboxGenomeTemplate<int> PermutationSboxGenome;

#endif /* BOXES_H_ */
