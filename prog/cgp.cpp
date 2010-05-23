/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * cgp.cpp
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
#include "cgp.h"

int CgpGenome::write(std::ostream & fout) const {
	chromozome l_chrom = p_chrom;
	fout << '{' << param_inputs << ',' << param_outputs << ',' << param_columns << ',' << param_rows << ',' <<
			block_in << ',' <<  l_back << ',' << blocksUsed() << '}';
	for(int i=0; i<outputidx; i++) {
		if (i % 3 == 0) fout << "([" << ((i/3)+param_inputs) << ']';
		fout << (*l_chrom++);
		fout << (((i+1) % 3 == 0) ? ")" : ",");
	}
	fout << '(';
	for(int i=outputidx; i<outputidx+param_outputs; i++) {
		if (i > outputidx) fout << ',';
		fout << (*l_chrom++);
	}
	fout << ')' << endl;
	return 0;
}

int CgpGenome::blocksUsed() const {
//	cout << "CgpGenome::BLOCKS_USED" << " " << algorithm() << endl;
//	cout << this << " : "<< p_chrom << endl;
    
    int used[maxidx_out];
    memset(used, 0, maxidx_out*sizeof(int));
    chromozome p_pom;
    
    //oznacit jako pouzite bloky napojene na vystupy
    p_pom = p_chrom + outputidx;
    for (int i=0; i < param_outputs; i++) {
//    	cout << p_pom << " : ";
    	int in = *p_pom++;
//    	cout << "in: "<< in << endl;
        used[in] = 1;
    }

    int idx = maxidx_out-1;
    int poc = 0;
    
    //pruchod od vystupu ke vstupum
    p_pom = p_chrom + outputidx - 1;
    for (int i=param_columns; i > 0; i--) {
        for (int j=param_rows; j > 0; j--,idx--) {
            p_pom--; //fce
            if (used[idx] == 1) { //pokud je blok pouzit, oznacit jako pouzite i bloky, na ktere je napojen
               int in = *p_pom--; //in2
               used[in] = 1;
               in = *p_pom--; //in1
               used[in] = 1;
               poc++;
            } else {
               p_pom -= block_in; //posun na predchozi blok
            }
        }
    }

    return poc;
}

inline int CgpGenome::mutation() {
	int genu = GARandomInt(1, getMutationMax());     //pocet genu, ktere se budou mutovat
	for (int j = 0; j < genu; j++) {
		int i =  GARandomInt(0, (outputidx + param_outputs) -1); //vyber indexu v chromozomu pro mutaci
		int column = (int) (i / columnSize);

		if (i < outputidx) { //mutace bloku
			if ((i % 3) < 2) { //mutace vstupu
				p_chrom[i] = algorithm()->getRandomInputForColumn(column);
			} else { //mutace fce
				p_chrom[i] = GARandomInt(0, functions-1);
			}
		} else { //mutace vystupu
			p_chrom[i] = GARandomInt(0, maxidx_out-1);
		}
	}
	_evaluated = gaFalse;
	return genu;
}

int CgpGenome::Mutate(GAGenome & g, float){
	CgpGenome & genome = (CgpGenome &) g;
	return genome.mutation();
}

inline void CgpGenome::randomize() {
//	cout << "CgpGenome::INIT" << " " << algorithm() << endl;
//	cout << this << " : "<< p_chrom << endl;
	chromozome p_pom = p_chrom;

	for (int j=0; j < param_columns*param_rows; j++) {
		int column = (int)(j / param_rows);
		// vstup 1
		*p_pom++ = algorithm()->getRandomInputForColumn(column);
		// vstup 2
		*p_pom++ = algorithm()->getRandomInputForColumn(column);
		// funkce
		*p_pom++ = GARandomInt(0, functions-1);
	}
	for (int j=outputidx; j < outputidx+param_outputs; j++)  //napojeni vystupu
		*p_pom++ = GARandomInt(0, maxidx_out-1);
}

void CgpGenome::Init(GAGenome & g){
	CgpGenome & genome = (CgpGenome &) g;
	genome.randomize();
}

#define CHROMOZOME_CMP (memcmp(genome1.p_chrom, genome2.p_chrom, (genome1.outputidx + genome2.param_outputs)*sizeof(chromozomDataType)))
float CgpGenome::Compare(const GAGenome & g1, const GAGenome & g2){
	if (&g1 == &g2) return 0.0;
	CgpGenome & genome1 = (CgpGenome &) g1;
	CgpGenome & genome2 = (CgpGenome &) g2;
	COMPARE_HELPER(outputidx)
	COMPARE_HELPER(param_outputs)
	COMPARE_HELPER(param_inputs)
	COMPARE_HELPER(param_columns)
	COMPARE_HELPER(param_rows)
	COMPARE_HELPER(l_back)
	COMPARE_HELPER(functions)
	int mcmp = CHROMOZOME_CMP;
//	report << genome1 << endl;
//	report << genome2 << endl;
//	report << mcmp << endl;
	return mcmp;
}

int CgpGenome::equal(const GAGenome & other) const {
	if (&other == this) return true;
	CgpGenome & genome1 = (CgpGenome &) *this;
	CgpGenome & genome2 = (CgpGenome &) other;
	EQUAL_HELPER(outputidx)
	EQUAL_HELPER(param_outputs)
	EQUAL_HELPER(param_inputs)
	EQUAL_HELPER(param_columns)
	EQUAL_HELPER(param_rows)
	EQUAL_HELPER(l_back)
	EQUAL_HELPER(functions)
	bool mcmp = !CHROMOZOME_CMP;
//	report << genome1 << endl;
//	report << genome2 << endl;
//	report << mcmp << endl;
	return mcmp;
}

int CgpGenome::output(int x) const {
	int *vystupy;
	vystupy = new int[maxidx_out+param_outputs];
    int *p_vystup = vystupy + param_inputs; //posunuti az za hodnoty vstupu
    chromozome p_pom = p_chrom;
    
    for (int i = 0; i < param_inputs; i++) {
    	vystupy[i] = (x & (1<<i)) > 0;
    }

    for (int i = 0; i < param_columns; i++) {  //vyhodnoceni funkce pro sloupec
    	for (int j = 0; j < param_rows; j++) { //vyhodnoceni funkce pro radky sloupce
    		int in1 = vystupy[*p_pom++];
    		int in2 = vystupy[*p_pom++];
    		int fce = *p_pom++;
    		switch (fce) {
				case 0: *p_vystup++ = in1; break;       //in1

				case 1: *p_vystup++ = in1 & in2; break; //and
				case 2: *p_vystup++ = in1 | in2; break; //or
				case 3: *p_vystup++ = in1 ^ in2; break; //xor
				
				case 4: *p_vystup++ = ~in1; break;  //not in1
				case 5: *p_vystup++ = ~in2; break;  //not in2
				
				case 6: *p_vystup++ = in1 & ~in2; break;
				case 7: *p_vystup++ = ~(in1 & in2); break;
				case 8: *p_vystup++ = ~(in1 | in2); break;
				default: *p_vystup++ = 0xffffffff; //log 1
    		}
    	}
    }

    int res = 0;
    for (int i = 0; i < getOutputsCount(); i++) {
    	res += (vystupy[*p_pom++]&1) << i;
    }
    delete[] vystupy;
    return res;
}

int CgpGenome::inputsCount() const {
	return getInputsCount();
}

int CgpGenome::outputsCount() const {
	return getOutputsCount();
}

float CgpGenome::Evaluate(GAGenome & g){
	assert(false);
	CgpGenome & genome = (CgpGenome &) g;
	return genome.param_rows*genome.param_columns + genome.param_outputs - genome.blocksUsed();
}

int CgpGenome::Cross(const GAGenome &, const GAGenome&, GAGenome*, GAGenome*){
	assert(false);
	return 0;
}

int CgpGenome::realizationEfficiency() const {
	return param_rows*param_columns - blocksUsed();
}

SboxSearchAlgorithmBase::SboxSearchAlgorithmBase(const GAGenome &g) : GASimpleGA(g) {
	if (typeid(g) == typeid(CgpGenome)) {
		CgpGenome &genome = (CgpGenome&) g;
		//-----------------------------------------------------------------------
		//Priprava pole moznych hodnot vstupu pro sloupec podle l-back a ostatnich parametru
		//-----------------------------------------------------------------------
		c_val.reserve(genome.getColumnsCount());

		for (int i=0; i < genome.getColumnsCount(); i++) {
			intVector columnValues;

			int minidx = genome.getRowsCount()*(i-genome.getLBack()) + genome.getInputsCount();
			if (minidx < genome.getInputsCount()) minidx = genome.getInputsCount(); //vystupy bloku zacinaji od param_in do param_in+m*n
			int maxidx = i*genome.getRowsCount() + genome.getInputsCount();

			uint count = genome.getInputsCount() + maxidx - minidx;
			columnValues.reserve(count);

			for (int k=0; k < genome.getInputsCount(); k++) //vlozeni indexu vstupu komb. obvodu
				columnValues.push_back(k);
			for (int k=minidx; k < maxidx; k++) //vlozeni indexu moznych vstupu ze sousednich bloku vlevo
				columnValues.push_back(k);

			assert(columnValues.size() == count);
			c_val.push_back(columnValues);
		}
	}
}
