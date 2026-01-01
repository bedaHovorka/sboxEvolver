/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * common.h
 *
 *  Created on: Mar 17, 2010
 *      Author: Bedrich Hovorka
 *
 * Sbox representation like SubCrumb from Luffa SHA3 candidate
 */
#ifndef SOFTWARESBOX_H_
#define SOFTWARESBOX_H_

#include "boxes.h"
#include <algorithm>

typedef enum {
	MOV,
	AND,
	OR,
	XOR,
	NOT,
	NOP
} TInstructionType;

struct TInstruction {
	TInstructionType type;
	short operand1;
	short operand2;

	explicit TInstruction(TInstructionType t, short o1, short o2) : type(t), operand1(o1), operand2(o2) {}
	TInstruction(const TInstruction &orig) : type(orig.type), operand1(orig.operand1), operand2(orig.operand2) {}

	TInstruction& operator=(const TInstruction& orig) {
		if(&orig != this) {
			type = orig.type;
			operand1 = orig.operand1;
			operand2 = orig.operand2;
		}
		return *this;
	}
};

typedef std::vector<TInstruction> InstructionVector;

class Cycle {
private:
	static const uint instructionsInCycle = 3;

	const bool isFirst;
	const bool isLast;
	InstructionVector instructions;

	inline static int randomOperand(const intVector &skiplist);
	inline static int randomOperand();
	inline static TInstructionType randomType(bool isFirst, bool isLast);

	explicit Cycle(bool first, bool last, const InstructionVector &vec) : isFirst(first), isLast(last), instructions(vec) {
		assert(!(first&&last));
		assert(vec.size() == instructionsInCycle);
	}
public:
	Cycle(const Cycle &orig) : isFirst(orig.isFirst), isLast(orig.isLast), instructions(orig.instructions) {}

	Cycle& operator=(const Cycle& orig) {
		if(&orig != this) {
			assert(isFirst == orig.isFirst);
			assert(isLast == orig.isLast);
			instructions = orig.instructions;
		}
		return *this;
	}

	static Cycle randomInstance(bool isFirst, bool isLast);
	int mutation(const float pMut);
	void compute(int **registers) const;

	int compareTo(const Cycle&) const;

	friend std::ostream& operator<<(std::ostream& output, const Cycle &cycle);
	static inline Cycle testCycle(const TInstruction &i1, const TInstruction &i2, const TInstruction &i3, bool first=false, bool last=false);
};

std::ostream& operator<<(std::ostream& output, const Cycle &cycle);

typedef std::vector<Cycle> CycleVector;

class SoftwareSboxGenome : public Sbox, public GAGenome {
public:
	GADefineIdentity("SoftwareSboxGenome", 296);
	static void Init(GAGenome&);
	static int Mutate(GAGenome&, float);
	static float Compare(const GAGenome&, const GAGenome&);
	static float Evaluate(GAGenome&);
	static int Cross(const GAGenome&, const GAGenome&, GAGenome*, GAGenome*);
	static SoftwareSboxGenome testBox();

	static const uint REGISTERS_COUNT = 5;
	static const uint CYCLES_COUNT = 6;
private:
	const int bitsPerRegister;
	CycleVector cycles;
public:
	SoftwareSboxGenome(int bitsPR) : GAGenome(Init, Mutate, Compare), bitsPerRegister(bitsPR) {
		evaluator(Evaluate);
		crossover(Cross);
	}

	SoftwareSboxGenome(const SoftwareSboxGenome &orig) : Sbox(orig), GAGenome(Init, Mutate, Compare), bitsPerRegister(orig.bitsPerRegister) {
		copy(orig);
	}

	SoftwareSboxGenome& operator=(const GAGenome& orig){
		assert(false);
		if(&orig != this) copy(orig);
		return *this;
	}

	virtual void copy(const GAGenome& o) {
		GAGenome::copy(o);
		SoftwareSboxGenome &orig = (SoftwareSboxGenome&) o;
		Sbox::copy(orig);
		assert(bitsPerRegister == orig.bitsPerRegister);
		assert(evaluator() == orig.evaluator());
		cycles = orig.cycles;
	}

	virtual GAGenome* clone(CloneMethod) const {return new SoftwareSboxGenome(*this);}

	inline void setCycles(const CycleVector &vector) {
		cycles = vector;
		assert(cycles.size() == CYCLES_COUNT);
		_evaluated = gaFalse;
	}

	inline const CycleVector & getCycles() const {
		return cycles;
	}

	virtual int output(int x) const;
	virtual int inputsCount() const;
	virtual int outputsCount() const;
	virtual int equal(const GAGenome &) const;
	virtual int write(std::ostream&) const;

	inline int mutate(float pMut);

	virtual const SboxPtrVector neigboursInStateSpace() const {assert(false); }//zatim netreba...
};


#endif /* SOFTWARESBOX_H_ */
