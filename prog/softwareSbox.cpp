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
#include "softwareSbox.h"

void SoftwareSboxGenome::Init(GAGenome& g) {
	SoftwareSboxGenome &genome = (SoftwareSboxGenome&) g;
	CycleVector cycles;
	cycles.reserve(CYCLES_COUNT);
	for (uint i = 0; i < CYCLES_COUNT; i++) cycles.push_back(Cycle::randomInstance(i == 0, i == CYCLES_COUNT-1));
	genome.setCycles(cycles);
}

inline int SoftwareSboxGenome::mutate(float pMut) {
	int sum = 0;
	for (CycleVector::iterator c = cycles.begin(); c != cycles.end(); c++) sum += c->mutation(pMut);
	if (sum) _evaluated = gaFalse;
	return sum;
}

int SoftwareSboxGenome::Mutate(GAGenome&g, float pMut) {
	SoftwareSboxGenome &genome = (SoftwareSboxGenome&) g;
	return genome.mutate(pMut);
}

float SoftwareSboxGenome::Compare(const GAGenome&g1, const GAGenome&g2) {
	if (&g1 == &g2) return 0.0;
	SoftwareSboxGenome & genome1 = (SoftwareSboxGenome &) g1;
	SoftwareSboxGenome & genome2 = (SoftwareSboxGenome &) g2;
	COMPARE_HELPER(cycles.size())
	for (CycleVector::const_iterator c1 = genome1.cycles.begin(), c2 = genome2.cycles.begin(); c1 != genome1.cycles.end(); c1++, c2++) {
		const int res = c1->compareTo(*c2);
		if (res) return res;
	}
	return 0;
}

float SoftwareSboxGenome::Evaluate(GAGenome&) {
	assert(false);
	return 0.0;
}

int SoftwareSboxGenome::Cross(const GAGenome&p1, const GAGenome&p2, GAGenome*c1, GAGenome*c2) {
	const CycleVector &parent1 = ((SoftwareSboxGenome&) p1).getCycles();
	const CycleVector &parent2 = ((SoftwareSboxGenome&) p2).getCycles();

	int n=0;
	int point = GARandomInt(1, CYCLES_COUNT-2);
	if(c1){
		SoftwareSboxGenome &sis=(SoftwareSboxGenome &)*c1;
		CycleVector cycles;
		cycles.insert(cycles.begin(), parent1.begin(), parent1.begin()+point);
		cycles.insert(cycles.begin()+point, parent2.begin()+point, parent2.end());
		sis.setCycles(cycles);
		n++;
	}

	if(c2){
		SoftwareSboxGenome &bro=(SoftwareSboxGenome &)*c2;
		CycleVector cycles;
		cycles.insert(cycles.begin(), parent2.begin(), parent2.begin()+point);
		cycles.insert(cycles.begin()+point, parent1.begin()+point, parent1.end());
		bro.setCycles(cycles);
		n++;
	}

	return n;
}

int SoftwareSboxGenome::output(int x) const {
	assert(!(inputsCount() & 3));

	int *r = new int[REGISTERS_COUNT];
	memset(r, 0, REGISTERS_COUNT*sizeof(int));

	const int mask = ((1<<bitsPerRegister)-1);
	for (int i=0; i < 4; i++) {
		r[i] = x&mask;
		x >>= bitsPerRegister;
	}

	for (CycleVector::const_iterator c = cycles.begin(); c != cycles.end(); c++) c->compute(&r);

	//vystup
	int result = 0;
	assert(outputsCount() == inputsCount());
	for (uint i=0, shift=0; i < REGISTERS_COUNT; i++) {
		if (i == 3) continue;
		result += (r[i]&mask) << shift;
		shift += bitsPerRegister;
	}
	delete[] r;
	return result;
}

int SoftwareSboxGenome::inputsCount() const {
	return 4 * bitsPerRegister;
}

int SoftwareSboxGenome::outputsCount() const {
	return 4 * bitsPerRegister;
}

int SoftwareSboxGenome::equal(const GAGenome &other) const {
	if (&other == this) return true;
	SoftwareSboxGenome & genome1 = (SoftwareSboxGenome &) *this;
	SoftwareSboxGenome & genome2 = (SoftwareSboxGenome &) other;
	EQUAL_HELPER(cycles.size())
	for (CycleVector::const_iterator c1 = genome1.cycles.begin(), c2 = genome2.cycles.begin(); c1 != genome1.cycles.end(); c1++, c2++) {
		const int res = c1->compareTo(*c2);
		if (res != 0) return false;
	}
	return true;
}

int SoftwareSboxGenome::write(std::ostream& output) const {
	for (CycleVector::const_iterator c = cycles.begin(); c != cycles.end(); c++) {
		if (c != cycles.begin()) output << " , ";
		output << '{' << *c << '}';
	}
	return 0;
}

SoftwareSboxGenome SoftwareSboxGenome::testBox() {
	// taken from Luffa SHA3 candidate
	// DPMax = 1/4, LPMax = 1/4
	// branching factor = 2
	CycleVector cycles;
	cycles.push_back(Cycle::testCycle(TInstruction(MOV, 4, 0), TInstruction(XOR, 2, 1), TInstruction(AND, 0, 1), true));
	cycles.push_back(Cycle::testCycle(TInstruction(XOR, 0, 2), TInstruction(NOT, 1, 1), TInstruction(OR, 2, 4)));
	cycles.push_back(Cycle::testCycle(TInstruction(XOR, 2, 3), TInstruction(XOR, 4, 0), TInstruction(AND, 3, 0)));
	cycles.push_back(Cycle::testCycle(TInstruction(XOR, 3, 1), TInstruction(NOT, 4, 0), TInstruction(OR, 1, 2)));
	cycles.push_back(Cycle::testCycle(TInstruction(XOR, 4, 1), TInstruction(XOR, 0, 3), TInstruction(AND, 1, 2)));
	cycles.push_back(Cycle::testCycle(TInstruction(XOR, 1, 3), TInstruction(NOP, 4, 0), TInstruction(NOP, 3, 0), false, true));

	SoftwareSboxGenome genome(1);
	genome.setCycles(cycles);
	return genome;
}

inline Cycle Cycle::testCycle(const TInstruction &i1, const TInstruction &i2, const TInstruction &i3, bool first, bool last) {
	InstructionVector cycleInstructions;
	cycleInstructions.push_back(i1);
	cycleInstructions.push_back(i2);
	cycleInstructions.push_back(i3);
	return Cycle(first, last, cycleInstructions);
}

inline int Cycle::randomOperand(const intVector &list) {
	intVector allowed;
	for (uint i=0; i < SoftwareSboxGenome::REGISTERS_COUNT; i++) {
		if (std::find(list.begin(), list.end(), i) == list.end()) allowed.push_back(i);
	}
	return allowed[GARandomInt(0, allowed.size()-1)];
}

inline int Cycle::randomOperand()
{
    return GARandomInt(0, SoftwareSboxGenome::REGISTERS_COUNT - 1);
}

inline TInstructionType Cycle::randomType(bool isFirst, bool isLast)
{
    return (TInstructionType)(GARandomInt(isFirst ? MOV : MOV + 1, isLast ? NOP : NOP - 1));
}

Cycle Cycle::randomInstance(bool isFirst, bool isLast) {
	InstructionVector instructions;
	intVector skipList;
	for (uint i = 0; i < Cycle::instructionsInCycle; i++) {
		const int randomOperand1 = randomOperand(skipList);
		instructions.push_back(TInstruction(randomType(isFirst, isLast), randomOperand1, randomOperand()));
		skipList.push_back(randomOperand1);
	}
	return Cycle(isFirst, isLast, instructions);
}

int Cycle::mutation(const float pMut) {
	intVector skipList;
	for (InstructionVector::const_iterator i = instructions.begin(); i != instructions.end(); i++) {
		skipList.push_back(i->operand1);
	}

	int m = 0;
	for (InstructionVector::iterator i = instructions.begin(); i != instructions.end(); i++) {
		if (!GAFlipCoin(pMut)) continue;

		int what = GARandomInt(1, 3);
		if (what == 1) {// zmena typu
			i->type = randomType(isFirst, isLast);
		} else if (what == 2) {// zmena operandu2
			i->operand2 = randomOperand();
		} else {// zmena operandu1
			int newOperand = randomOperand(skipList);
			std::remove(skipList.begin(), skipList.end(), i->operand1);
			skipList.push_back(newOperand);
			i->operand1 = newOperand;
		}
		m++;
	}
	return m;
}

#define COMPUTE_OPERATION(op) {(*registers)[i->operand1] op previousValues[i->operand2];}

void Cycle::compute(int **registers) const {
	// ulozit predchozi hodnoty registru
	int previousValues[SoftwareSboxGenome::REGISTERS_COUNT];
	memcpy(previousValues, *registers, SoftwareSboxGenome::REGISTERS_COUNT*sizeof(int));

	for (InstructionVector::const_iterator i = instructions.begin(); i != instructions.end(); i++) {
		switch (i->type) {
			case MOV:
				COMPUTE_OPERATION(=)
				break;

			case AND:
				COMPUTE_OPERATION(&=)
				break;

			case OR:
				COMPUTE_OPERATION(|=)
				break;

			case XOR:
				COMPUTE_OPERATION(^=)
				break;

			case NOT:
				COMPUTE_OPERATION(= ~)
				break;

			case NOP:
				break;

			default:
				assert(false);
				break;
		}
	}
}

#define CMP_INSTR_HELPER(param) {int cmp = instr1.param - instr2.param; if (cmp) return cmp;}
inline int compareInstruction(const TInstruction &instr1,const TInstruction &instr2) {
	CMP_INSTR_HELPER(type)
	CMP_INSTR_HELPER(operand1)
	CMP_INSTR_HELPER(operand2)
	return 0;
}

int Cycle::compareTo(const Cycle& other) const {
	assert (instructions.size() == other.instructions.size());
	int cmp = isFirst - other.isFirst; if (cmp) return cmp;
	cmp = isLast - other.isLast; if (cmp) return cmp;
	for (InstructionVector::const_iterator i1 = instructions.begin(), i2 = other.instructions.begin(); i1 != instructions.end(); i1++, i2++) {
		cmp = compareInstruction(*i1, *i2); if (cmp) return cmp;
	}
	return 0;
}

std::ostream& operator<<(std::ostream& output, const Cycle &cycle) {
	for (InstructionVector::const_iterator i = cycle.instructions.begin(); i != cycle.instructions.end(); i++) {
		if (i != cycle.instructions.begin()) output << " ; ";

		switch (i->type) {
			case MOV:
				output << "MOV";
				break;

			case AND:
				output << "AND";
				break;

			case OR:
				output << "OR";
				break;

			case XOR:
				output << "XOR";
				break;

			case NOT:
				output << "NOT";
				break;

			case NOP:
				output << "NOP";
				break;

			default:
				assert(false);
				break;
		}

		if (i->type != NOP) {
			output << " r" << i->operand1;
			if (i->type != NOT) output << ", " << 'r' << i->operand2;
		}
	}
	return output;
}

