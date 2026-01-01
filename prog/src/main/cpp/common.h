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
 *  Created on: Feb 23, 2010
 *      Author: Bedrich Hovorka
 *
 * Common routines
 */
#ifndef SBOXEVOLUTION_COMMON
#define SBOXEVOLUTION_COMMON

#include <ga/garandom.h>
#include <cassert>

extern "C" {
	#include <Python.h>
}

#include <cmath>
#include <typeinfo>
#include <vector>
#include <algorithm>

#include <ga/std_stream.h>
#include <ga/GASimpleGA.h>
#include <ga/GA1DBinStrGenome.h>
#include <ga/GA1DArrayGenome.h>

#define report STD_COUT << std::boolalpha << __FILE__ << ':' << __LINE__ << "\t::\t"
#define endl std::endl
#define COMPARE_HELPER(property) {int cmp = genome1.property - genome2.property; if (cmp != 0) return cmp;}
#define EQUAL_HELPER(property) {if (genome1.property != genome2.property) return false;}

typedef std::vector<int> intVector;

inline uint hammingWeight(int n) {
	return __builtin_popcount(n);
}

template <typename Numb> inline bool is_odd(Numb s) {
	return s&1;
}

inline int binaryDot(int x, int y) {
	// * == & paralelne ;)
	// suma - suda/licha hammingova vaha
	return is_odd(hammingWeight(x & y));
}

inline PyObject* createPythonString(const std::string & str) {
	// Python 3 uses PyUnicode for string objects (PyString removed)
	return PyUnicode_FromStringAndSize(str.c_str(), str.size());
}

template <typename PtrType > inline void freeAndNULL(PtrType *ptr) {
//	assert(false);
	if (ptr) {
		delete ptr;
		ptr = NULL;
	}
}

template <typename Titer > inline void deleteMembers(Titer begin, Titer end) {
	for (Titer i = begin; i != end; i++) {
		freeAndNULL(*i);
	}
}

inline GABool toGABool(bool b) {
	return (b) ? gaTrue : gaFalse; // pro poradek...
}

class SboxSearchAlgorithmBase : public GASimpleGA {
private:
	// kvuli CgpGenomu...
	typedef std::vector<intVector> PrecomputedOutputs;
	PrecomputedOutputs c_val;
public:
	explicit SboxSearchAlgorithmBase(const GAGenome &g);
	virtual ~SboxSearchAlgorithmBase() {}
	virtual const GAPopulation &bestResults() {
		return statistics().bestPopulation();
	}

	inline int getRandomInputForColumn(int column) {
		return c_val[column][GARandomInt(0, (c_val[column].size())-1)];
	}
};


#endif
