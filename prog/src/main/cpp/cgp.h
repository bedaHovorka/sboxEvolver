/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * cgp.h
 *
 *  Created on: Mar 1, 2010
 *      Author: Bedrich Hovorka
 *
 * Implementation of Cartesian genetic programing
 *
 * Based on Tools4CGP by Zdenek Vasicek and Lukas Sekanina
 * http://www.fit.vutbr.cz/research/view_product.php?id=61&notitle=1
 * http://www.fit.vutbr.cz/%7Evasicek/cgp
 */
#ifndef SBOXEVOLUTION_CGP
#define SBOXEVOLUTION_CGP

#include "boxes.h"
#include <string>
#include <ga/GAGenome.h>

typedef int chromozomDataType;
typedef chromozomDataType *chromozome;//dynamicke pole int, velikost dana m*n*(vstupu bloku+vystupu bloku) + vystupu komb

class ParallelRandomSearchAlgorithm;

class CgpGenome : public Sbox, public GAGenome {
public:
	GADefineIdentity("CGPGenome", 206);
	static void Init(GAGenome&);
	static int Mutate(GAGenome&, float);
	static float Compare(const GAGenome&, const GAGenome&);
	static float Evaluate(GAGenome&);
	static int Cross(const GAGenome&, const GAGenome&, GAGenome*, GAGenome*);
private:
	const int block_in;
	const int param_inputs, param_outputs, param_columns, param_rows, mutationMax, l_back, functions;
	int outputidx, maxidx_out, columnSize;

	chromozome p_chrom;

	int init () {
		columnSize = param_rows*(block_in+1); //pocet polozek ktery zabira sloupec v chromozomu
		outputidx   = param_columns*columnSize; //index v poli chromozomu, kde zacinaji vystupy
		maxidx_out  = param_rows*param_columns + param_inputs; //max. index pouzitelny jako vstup  pro vystupy

		int chromSize = (outputidx + param_outputs);
		if (!p_chrom) {
			p_chrom = new chromozomDataType[chromSize];
		}
		return chromSize;
	}

	inline void randomize();
public:
	CgpGenome(int in, int out, int m, int n, int mutMax, int l=1, int f=9) : GAGenome(Init, Mutate, Compare), block_in(2),
		param_inputs(in), param_outputs(out), param_columns(m), param_rows(n), mutationMax(mutMax), l_back(l), functions(f), p_chrom(NULL) {
		evaluator(Evaluate);
		crossover(Cross);

		int chromSize = init();
		memset(p_chrom, 0, chromSize * sizeof(chromozomDataType));
	}


	CgpGenome(const CgpGenome& orig) : Sbox(orig), GAGenome(Init, Mutate, Compare), block_in(orig.block_in), param_inputs(orig.param_inputs),
			param_outputs(orig.param_outputs), param_columns(orig.param_columns), param_rows(orig.param_rows), mutationMax(orig.mutationMax),
			l_back(orig.l_back), functions(orig.functions), p_chrom(NULL) {
		copy(orig);
	}

	virtual ~CgpGenome() {
		delete[] p_chrom;
	}

	CgpGenome& operator=(const GAGenome& orig){
		throw std::runtime_error("CgpGenome::operator= not implemented");
		if(&orig != this) copy(orig);
		return *this;
	}

	virtual GAGenome* clone(CloneMethod) const {return new CgpGenome(*this);}

	virtual void copy(const GAGenome& o) {
		GAGenome::copy(o);
		CgpGenome & orig = (CgpGenome &) o;
		Sbox::copy(orig);
		int chromSize = init();
		memcpy(p_chrom, orig.p_chrom, chromSize * sizeof(chromozomDataType));
	}

	virtual int equal(const GAGenome &) const;
	virtual int write(std::ostream&) const;
	virtual int output(int) const;
	virtual int inputsCount() const;
	virtual int outputsCount() const;
	virtual int realizationEfficiency() const;

	inline int getInputsCount() const {
		return param_inputs;
	}

	inline int getOutputsCount() const {
		return param_outputs;
	}

	inline int getColumnsCount() const {
		return param_columns;
	}

	inline int getRowsCount() const {
		return param_rows;
	}

	inline int getLBack() const {
		return l_back;
	}

	inline int getFunctionsCount() const {
		return functions;
	}

	inline int getMutationMax() const {
		return mutationMax;
	}

	inline int mutation();

	virtual const SboxPtrVector neigboursInStateSpace() const { throw std::runtime_error("neigboursInStateSpace not implemented for CgpGenome"); }//cgp je special, zde netreba

protected:
	inline SboxSearchAlgorithmBase * algorithm() const {
		return (SboxSearchAlgorithmBase *) geneticAlgorithm();
	};

	int blocksUsed() const;
};

#endif
