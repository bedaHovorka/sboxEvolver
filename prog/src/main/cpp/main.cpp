/* Brno University of Technology
 *
 * Faculty of Information Technology
 *
 * MSc Thesis       2009/2010
 *
 *
 * Design of S-Boxes Using Genetic Algorithms
 *
 * main.cpp
 *
 *  Created on: Feb 18, 2010
 *      Author: Bedrich Hovorka
 *
 * Declaration of criterions combinations
 */
#include "main.h"

inline void stopOnError(std::string message) {
	throw std::runtime_error(message);
}

void setParameters(SboxSearchAlgorithmBase & ga, const int popsize, const int ngen, const float pmut, const float pcross,
		const int bestGenomes)
{
	ga.populationSize(popsize);
	ga.nGenerations(ngen);
	ga.nBestGenomes(bestGenomes);
	ga.nConvergence(ngen>100 ? ngen/10 : 10); // i kdyz nevim zda neodvozovat od popsize, ci jestli nenastavovat misto neho
	ga.pMutation(pmut);
	ga.pCrossover(pcross);
}

// statistiky budou uchovavat popSize nejlepsich vysledku, avsak nahodne prohledavani ma popSize 0, tak je treba dospecikovat zvlast
inline void setParameters(SboxSearchAlgorithmBase & ga, const int popsize, const int ngen, const float pmut, const float pcross) {
	setParameters(ga, popsize, ngen, pmut, pcross, popsize);
}

TSearching newParallelRandomSearching(TGenome genome, int popsize, int ngen, float pMut) {
	ParallelRandomSearchAlgorithm *ga = new ParallelRandomSearchAlgorithm(*genome.ptr);
	setParameters(*ga, popsize, ngen, pMut, 0.0);

	TSearching searching = {ga};
	return searching;
}

TSearching newEdaSearching(TGenome g, int popsize, int ngen, bool bdma) {
	EstimationOfDistributionAlgorithm *ga = new EstimationOfDistributionAlgorithm(*g.ptr, bdma, popsize);
	setParameters(*ga, popsize, ngen, 0.0, 0.0);

	TSearching searching = {ga};
	return searching;
}

TSearching newGaSearching(TGenome g, int popsize, int ngen, float pMut, float pCross, bool elitism) {
	SboxSearchAlgorithmBase *ga = new SboxSearchAlgorithmBase(*g.ptr);
	setParameters(*ga, popsize, ngen, pMut, pCross);
	ga->elitist(toGABool(elitism));

	TSearching searching = {ga};
	return searching;
}

void ParallelRandomSearchAlgorithm::step() {
	int n = 0;
	const int popSize = pop->size();
	for (; n < popSize && oldBestGenome && pop->individual(n).equal(*oldBestGenome); n++);// neshoduje se predchozim nejlepsim
	GAGenome *bestGenome = (n < popSize) ? pop->individual(n).clone() : pop->individual(0).clone();

	pop->size(0);
	for (int i=0; i<popSize; i++) {
		GAGenome *child = bestGenome->clone();
		stats.nummut += child->mutate(pmut);
		pop->add(child);
	}

	pop->evaluate();
	stats.update(*pop);

	freeAndNULL(oldBestGenome);
	oldBestGenome = bestGenome;
}

void RandomSearchAlgorithm::step()
{
	GAGenome &actual = pop->individual(0);
	stats.nummut += actual.mutate(pmut);
	stats.update(*pop);
}

// u nahodneho prohledavani se neda hovorit o velikosti populace, ale
// toto cislo se da pouzit k nastaveni kolik nejlepsich jedincu se ma ulozit
TSearching newRandomSearching(TGenome g, int bestGenomes, int ngen) {
	RandomSearchAlgorithm *ga = new RandomSearchAlgorithm(*g.ptr);
	setParameters(*ga, 1, ngen, 0.1, 0, bestGenomes);
	TSearching searching = {ga};
	return searching;
}

void HeuristicSearchAlgorithm::evolve(unsigned int seed){
	initialize(seed);
	GAPopulation &open = *pop;

	uint n = 0;
	while(open.size() > 0 && !done()) {
		GAGenome &currentGenome = open.best(0);
		Sbox &currentSbox = dynamic_cast<Sbox &>(currentGenome);
		open.remove(&currentGenome);
		if ((uint)open.size() > maxStates) open.size(maxStates);
		SboxPtrVector neighbours = currentSbox.neigboursInStateSpace();
		for (SboxPtrVector::iterator i = neighbours.begin(); i != neighbours.end(); i++) {
			GAGenome *gAGenome = dynamic_cast<GAGenome*>(*i);
			open.add(gAGenome);
		}

		open.evaluate();
		open.scale();
		stats.update(open);
		n++;
	}
}

TSearching newHeuristicSearching(TGenome g, uint maxStates, uint ngen) {
	HeuristicSearchAlgorithm *ga = new HeuristicSearchAlgorithm(*g.ptr, maxStates);
	setParameters(*ga, 1, maxStates, ngen, 0);
	TSearching searching = {ga};
	return searching;
}

inline const CriterionsSet createCriterionsSet(const int criterionsCount, const CriterionsFitness *criterions) {
	CriterionsSet criterionsSet;
	criterionsSet.insert(criterions, criterions+criterionsCount);
	assert (criterionsSet.size() <= uint(criterionsCount) && criterionsCount > 1);
	return criterionsSet;
}

TSearching newVegaSearching(TGenome g, int popsize, int ngen, float pMut, float pCross, const int criterionsCount, const CriterionsFitness *criterions) {
	VegaAlgorithm *ga = new VegaAlgorithm(*g.ptr, createCriterionsSet(criterionsCount, criterions));
	setParameters(*ga, popsize, ngen, pMut, pCross);
	TSearching searching = {ga};
	return searching;
}

TSearching newSpeaSearching(TGenome g, int popsize, int ngen, float pMut, float pCross, const int criterionsCount, const CriterionsFitness *criterions) {
	SpeaAlgorithm *ga = new SpeaAlgorithm(*g.ptr, createCriterionsSet(criterionsCount, criterions));
	setParameters(*ga, popsize, ngen, pMut, pCross);
	TSearching searching = {ga};
	return searching;
}

TGenome privateCreateGenome(GenomeType type, GAGenome::Evaluator evaluator, const intVector & arguments) {
	TGenome genome;
	switch (type) {
	case BINARY:
		check(arguments.size() == 2, "Binary genome needs 2 specific arguments (number of inputs, number of outputs)");
		genome.ptr = new BinarySboxGenome(arguments[0], arguments[1], evaluator);
		break;
	case CGP:
	{
		check(arguments.size() == 5, "Binary genome needs 5 specific arguments (number of inputs, number of outputs, number of colums, number of rows, max mutations)");
		// inputsCount, outputsCount, columnsCount, rowsCount
		CgpGenome *cgpGenome = new CgpGenome(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		cgpGenome->evaluator(evaluator);
		genome.ptr = cgpGenome;
		break;
	}

	case SOFTWARE_IMPL:
	{
		check(arguments.size() == 1, "i686 genom needs one argument, number of quaternion of bits");
		SoftwareSboxGenome *softwareSboxGenome = new SoftwareSboxGenome(arguments[0]);
		softwareSboxGenome->evaluator(evaluator);
		genome.ptr = softwareSboxGenome;
		break;
	}

	case PERMUTATION:
		check(arguments.size() == 3, "Permutaion genom needs three arguments (bijective, number of inputs, number of outputs)");
		genome.ptr = new PermutationSboxGenome(arguments[0], arguments[1], arguments[2], evaluator);
		break;

	default:
		stopOnError("Wrong type of genome specified");
		break;
	}

	return genome;
}

#define VARARGS_TO_VECTOR va_list args;\
intVector arguments;\
arguments.reserve(argc);\
va_start(args, argc);\
for (int i=0; i < argc; i++) arguments.push_back(va_arg(args, int));\
va_end(args);\

TGenome createGenome(GenomeType type, CriterionsFitness criterion, int argc, ...) {
	VARARGS_TO_VECTOR

	GAGenome::Evaluator evaluator = criterionsEnumToFunction(criterion);
	TGenome privateCreateGenome0 = privateCreateGenome(type, evaluator, arguments);
	/*report << privateCreateGenome0.ptr << endl;*/
	return privateCreateGenome0;
}

TGenome createSymbolicalRegresionGenome(GenomeType type, int *expectedOutputs, int argc, ...) {
	VARARGS_TO_VECTOR

	GAGenome::Evaluator evaluator = symbolicalRegresionObjective;
	TGenome genome = privateCreateGenome(type, evaluator, arguments);
	Sbox *sbox = dynamic_cast<Sbox*>(genome.ptr);
	intVector *outputs = new intVector(expectedOutputs, expectedOutputs+(1<<sbox->inputsCount()));
	sbox->setExpectedValues(outputs);
	return genome;
}

void processSearching(TSearching searching, TerminatorCondition condition, uint seed) {
	GAGeneticAlgorithm *ga = searching.algorithm;
	switch (condition) {
		case GENERATION:
			ga->terminator(GAGeneticAlgorithm::TerminateUponGeneration);
			break;
		case CONVERGENCE:
			ga->terminator(GAGeneticAlgorithm::TerminateUponConvergence);
			break;
		case POPCONVERGENCE:
			ga->terminator(GAGeneticAlgorithm::TerminateUponPopConvergence);
			break;
		default:
			report << condition << endl;
			stopOnError("Wrong type of terminator specified");
			break;
	}
	ga->evolve(seed);
}

void parallelProcessSearching(const int taskCount, const TSearching tasks[], TerminatorCondition condition, const uint seeds[]) {
	#pragma omp parallel for schedule(guided) default(shared)
	for (int i = 0; i < taskCount; i++) {
		processSearching(tasks[i], condition, seeds[i]);
	}
}



inline void statisticsLikePing(std::ostringstream & out, const GAPopulation & population)
{
	out << "fitness min/avg/max/mdev : " <<  population.fitmin() << '/' << population.fitave() << '/';
	out << population.fitmax() << '/' << population.fitdev() << endl;
}

inline void simpleReportAboutGenome(std::ostream & out, GAGenome & individual) throw()
{
	out << std::setprecision(2) << individual.evaluate(gaTrue) << '\t';
	try {
		Sbox & sbox = dynamic_cast<Sbox& >(individual);

		const int inputsCount = sbox.inputsCount();
		const int outputsCount = sbox.outputsCount();
		const intVector & function = sbox.computeOutputs();
		out << bentScore(inputsCount, outputsCount, function) << '\t';
		out << strictAvelancheCriterion(inputsCount, outputsCount, function) << '\t';
		out << computeLPMax(function) << '\t';
		out << computeDPMax(function) << '\t';
		out << isBijective(function) << '\t';
		out << isBidirectional(function) << '\t';
		out << computeBranchingFactor(function) << '\t';
//        out << lagrangePolynomialDegree(function, outputsCount) << '\t';
		out << sbox.computeCriterionsFunction(POLYNOMIAL_DEGREE, function) << '\t';
		out << polynomialToString(computePolynomial(function)) << endl;

		out << "outputs:\t";
		for (intVector::const_iterator i = function.begin(); i != function.end(); i++) {
			out << *i << ' ';
		}
	} catch(std::bad_cast e) {
		for(int i = 0;i < 9;i++)
			out << "-\t";

	}
	out << endl << "Chromozome:\t" << individual << endl << endl;
}

PyObject* simpleReportSearching(TSearching searching) {
	SboxSearchAlgorithmBase *ga = searching.algorithm;
	GAPopulation population = ga->bestResults();
	std::ostringstream out;
	out << "convergence: " << ga->statistics().convergence() << endl;
	out << population.size() << " best results: "; statisticsLikePing(out, population);

	out << "Fit\tBent\tSAC\tLpMax\tDpMax\tBij\tBidir\tBF\tMinDeg\tPolynoms" << endl;
	for (int i=0; i < population.size(); i++) {
		GAGenome & individual = population.individual(i);
		simpleReportAboutGenome(out, individual);
	}

	return createPythonString(out.str());
}

void testSoftwareBox() {
	SoftwareSboxGenome genome = SoftwareSboxGenome::testBox();
	genome.evaluator(bentAndMosacObjective);
	simpleReportAboutGenome(std::cout, genome);
}

void closeSearching(TSearching searching) {
	freeAndNULL(searching.algorithm);
}

void setMiniMaxiSearching(TSearching searching, bool minimize) {
	searching.algorithm->minimaxi(minimize ? GAGeneticAlgorithm::MINIMIZE : GAGeneticAlgorithm::MAXIMIZE);
}

inline void arrayConv(const GAPopulation &population, float *&result) {
	for (int i = 0; i < population.size(); i++) {
		result[i] = population.individual(i).score();
	}
}

TStatistics getStatisticsSearching(TSearching searching, float *bestPopulationScores, float *bestPopulationCriterionsValues, int *bestPopulationOutputs) {
	const GAStatistics &stats = searching.algorithm->statistics();
	// nepouzije se posledni generace, ale populace nejlepsich jedincu ze statistik
	// (nejen proto, aby se dalo srovnavat s nahodnym prohledavanim)
	const GAPopulation &population = searching.algorithm->bestResults();
	TStatistics result;
	result.online = stats.online();
	result.offlineMax = stats.offlineMax();
	result.offlineMin = stats.offlineMin();

	arrayConv(population, bestPopulationScores);
	for (int i = 0; i < population.size(); i++) {
		Sbox & sbox = dynamic_cast<Sbox &>( population.individual(i));
		const intVector &outputs = sbox.computeOutputs();
		for (int j= BENT_AND_MOSAC; j < LAGRANGE; j++){
			bestPopulationCriterionsValues[i * LAGRANGE + j] = sbox.computeCriterionsFunction(CriterionsFitness(j), outputs);
		}

		const int outputsCount = outputs.size();
		for (int o = 0; o < outputsCount; o++) {
			bestPopulationOutputs[i * outputsCount + o] = outputs[o];
		}
	}

	result.maxEver = stats.maxEver();
	result.minEver =  stats.minEver();
	result.generation = stats.generation();
	result.convergence = stats.convergence();
	result.selections = stats.selections();
	result.crossovers = stats.crossovers();
	result.mutations = stats.mutations();
	result.replacements = stats.replacements();
	result.nBestGenomes = population.size();
	return result;
}

PyObject* bestPopulationStringsSearching(TSearching searching) {
	const GAPopulation &population = searching.algorithm->bestResults();
	PyObject* list = PyList_New(population.size());
	for (int i = 0; i < population.size(); i++) {
		std::ostringstream out;
		out << population.individual(i);
		PyList_SET_ITEM(list, i, createPythonString(out.str()));
	}
	return list;
}

//kdyby se standardni coredump nehodil...
//void terminateWithStackTrace() {
//	void *array[20];
//	size_t size;
//
//	// get void*'s for all entries on the stack
//	size = backtrace(array, 20);
//
//	// print out all the frames to stderr
//	backtrace_symbols_fd(array, size, 2);
//	exit(1);
//}
//
//void handler(int sig) {
//	fprintf(stderr, "Error: signal %d:\n", sig);
//	terminateWithStackTrace();
//}
//
void initialization() {
//	errno = 0;
//	signal(SIGABRT, handler);
//	if (errno) {
//		perror("Abort handler setting");
//		exit(1);
//	}
//	signal(SIGSEGV, handler);
//	if (errno) {
//		perror("Segmentation fault handler setting");
//		exit(1);
//	}
////	std::set_terminate(terminateWithStackTrace);
}
