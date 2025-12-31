#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
      Brno University of Technology
      Faculty of Information Technology

      MSc Thesis       2009/2010

      Design of S-Boxes Using Genetic Algorithms

      Created on May 8, 2010

      @author: Bedrich Hovorka
'''
import sys, numpy, shutil
from operator import itemgetter
from evolution import *
from pylab import *
from datetime import datetime
from random import shuffle

now = datetime.now()
OUTPUT_DIR = "./results%s"%(now.strftime("%Y%m%d%H%M%S"))
OUTPUT_IMAGE_FORMAT = "eps"

allCriterions = [CriterionFunction.BENT_AND_MOSAC, CriterionFunction.LP_MAX, CriterionFunction.DP_MAX,
                  CriterionFunction.SAC, CriterionFunction.BF, CriterionFunction.POLYNOMIAL_DEGREE]

runsCount = 40
popSize = 1000
generations = 50
# nasledujici cisla experimentu se shoduji s cisly podkapitoly

def experiment6():
    outputs = 3
    pMut = 0.1
    trainingSet = range(2**outputs)
    shuffle(trainingSet)
    print trainingSet

    genome = SymbolicalRegresionGenome(ChromozomeType.CGP, trainingSet, outputs, outputs, outputs, outputs-1, outputs*(outputs-1)/2)
    #genome = SymbolicalRegresionGenome(ChromozomeType.SOFTWARE_IMPL, trainingSet, outputs/4)

    def symbSearch(x):
        sr = Searching("ParallelRandom", genome, popSize, generations, pMut)
        sr.setMiniMaxi(True);
        return sr
    solutionValues, solutionStrings = runsWithStrings(map(symbSearch, xrange(runsCount)))
    print numpy.min(solutionValues)

def randomPermutations(): # nahodne prohledavani pro porovnani s sboxy (8x8) z experimentu 5
    callBacks = []
    for genomeFunction in [CriterionFunction.LP_MAX, CriterionFunction.DP_MAX]:
        s = lambda x : Searching("Random", Genome(ChromozomeType.PERMUTATION, genomeFunction, True, 8, 8), popSize, generations)
        callBacks.append(s)
    callBacksRunsNondominance(callBacks, allCriterions)
    return 0

def callBacksRunsNondominance(searchingCallBacks, criterions):
    for callBack in searchingCallBacks:
        criterionsValues, solutionValues, solutionStrings = multicriterialRuns(map(callBack, xrange(runsCount)), criterions)
        x = reshape(arange(0, len(solutionValues)), (1, len(solutionValues)))
        nondominanceValues = nondominance(numpy.concatenate((solutionValues, x.T), axis=1), criterions)

        print callBack(0)
        for i in nondominanceValues:
            print i
            print solutionStrings[int(i[-1])]
        print
        sys.stdout.flush()


def experiment5():
    criterions = allCriterions
    dimensions = [(6, 6), (8, 8)] # (4, 4), (6, 4),
    for inputs, outputs in dimensions:
        permutationGenome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.NONE_FITNESS, inputs == outputs, inputs, outputs)
        vegaPermutationSearching = lambda x: Searching("Vega", permutationGenome, popSize, generations, 0.1, 0.9, criterions)
        speaPermutationSearching = lambda x: Searching("Spea", permutationGenome, popSize, generations, 0.9, 0.05, criterions)

        cgpGenome = Genome(ChromozomeType.CGP, CriterionFunction.NONE_FITNESS, inputs, outputs, inputs, inputs-1, inputs*(inputs -1 )/2)
        vegaCgpSearching = lambda x: Searching("Vega", cgpGenome, popSize, generations, 0.9, 0.0, criterions)

        searchingCallBacks = [vegaCgpSearching, speaPermutationSearching, vegaPermutationSearching]
        if inputs == outputs and inputs % 4 == 0 :
            swGenome = Genome(ChromozomeType.SOFTWARE_IMPL, CriterionFunction.NONE_FITNESS, inputs/4)
            searchingCallBacks.append(lambda x: Searching("Vega", swGenome, popSize, generations, 0.01, 0.9, criterions))
            searchingCallBacks.append(lambda x: Searching("Spea", swGenome, popSize, generations, 0.05, 0.7, criterions))

        callBacksRunsNondominance(searchingCallBacks, criterions)

def experiment4():
    criterions = allCriterions + [CriterionFunction.BIJECTIVE_SCORE]
    #genome = Genome(ChromozomeType.SOFTWARE_IMPL, CriterionFunction.NONE_FITNESS, 1)
    genome = Genome(ChromozomeType.CGP, CriterionFunction.NONE_FITNESS, 4, 4, 4, 3, 6)
    #genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.NONE_FITNESS, True, 4, 4)
    for algorithm in ["Vega", "Spea"]:
        print algorithm
        results = {}
        for pMut in [0.01, 0.05]+[x/10. for x in xrange(10)]:
            for pCross in [0.0]:
            #for pCross in [0.01, 0.05]+[x/10. for x in xrange(10)]:
                func = lambda x: Searching(algorithm, genome, popSize, generations, pMut, pCross, criterions)
                criterionsValues, solutionValues, solutionStrings = multicriterialRuns(map(func, xrange(runsCount)), criterions)
                results[(pMut, pCross)] = criterionsValues, solutionValues

        #prumerny jedinec
        averageSolution = [0 for i in xrange(CRITERIONS_COUNT)]
        for criterion in criterions:
            averageSolution[criterion.ordinal] =  numpy.floor(10*numpy.mean(numpy.concatenate([i[0][criterion] for i in results.itervalues()])))/10
        print averageSolution
        aboveAverageCounts = {}

        # pocet ve vsech nadprumernych jedincu (pocet vsech, kteri slabe dominuji prumernemu jedinci)
        # tento system se pouziva na skolach - nejlepsi jsou jen ti co maj vedomosti ze vseho nad prumerem
        max = (0, 0)
        maxCount = 0
        for (pMut, pCross), (criterionsValues, solutionValues) in results.iteritems():
            # a nezalezi z kolika nBestGenomes (prave ze by se meli nashlukovat ti dobri, kdyz jich u SPEA bude min)
            aboveAverage = filter(lambda solution: weakDominatingOver(solution, averageSolution, criterions), solutionValues)
            count = len(aboveAverage)
            aboveAverageCounts[(pMut, pCross)] = count
            if count > maxCount:
                max = (pMut, pCross)
                maxCount = count
        print max

        keys = sorted(filter(lambda ((pMut, pCross), count) : count > 0, aboveAverageCounts.iteritems()))
        print keys
        if len(keys) > 0:
            figure()
            ind = numpy.arange(len(keys))
            width = 0.35
            xlabel("Probabilities - %s"%(algorithm))
            ylabel("Count")
            xticks(ind + width/2.0, map(lambda x: "(%.2g,%.1f)"%(x[0][0], x[0][1]), keys))
            bar(ind, map(lambda key: aboveAverageCounts[key[0]], keys), width)
            savefig("%s/%s.%s"%(OUTPUT_DIR, "%sParameters"%algorithm, OUTPUT_IMAGE_FORMAT))

            print("\\begin{table}[tb]\n\\begin{center}")
            print("\\begin{tabular}{| l | p{0.1\\textwidth} | %s}\n\\hline"%(reduce(lambda s, x: s + " p{0.1\\textwidth} |", criterions, '')))

            print("Parametry & Počet nadprůměrných %s \\\\ \\hline"%(reduce(lambda s, x: s + " & %s"%(x), criterions, '')))

            for key in keys:
                rowStr = reduce(lambda s, x: s + " & $%g$"%(runsStats(results[key[0]][0][x])[3]), criterions, '')
                print(" (%g, %g) & %d %s \\\\ \\hline" %(key[0][0], key[0][1], aboveAverageCounts[key[0]], rowStr))
            print("\\end{tabular}\n\\caption{Průměry při %s TODO}\n\\label{tab%sParameters}\n\\end{center}\n\\end{table}" % (algorithm, algorithm))

def experiment3():
    for genomeType in [ChromozomeType.CGP, ChromozomeType.SOFTWARE_IMPL]:
        criterionBests = {}
        for criterion in allCriterions:
            if genomeType == ChromozomeType.SOFTWARE_IMPL:
                genome = Genome(genomeType, criterion, 1)
            elif genomeType == ChromozomeType.CGP:
                genome = Genome(genomeType, criterion, 4, 4, 4, 3, 6)
            else:
                assert False

            results = {}
            for pMut in [0.01, 0.05]+[x/10. for x in xrange(10)]:
                func = lambda x: Searching("ParallelRandom", genome, popSize, generations, pMut)
                results[pMut] = runs(map(func, xrange(runsCount)))

            #boxplot
            algorithmsComparisonBoxPlotSorted("cgpVsLuffa%s"%(genomeType), criterion, results, "Pravdepodobnost mutace")

            func = lambda x: Searching("Random", genome, popSize, generations)
            randomSearchResult = runs(map(func, xrange(runsCount)))

            stats = []
            for pMut, values in results.iteritems():
                stats.append(runsStats(values) + [pMut])
            stats.append(runsStats(randomSearchResult) + ["Random"])
            # zarucena stabilita, takze pokud je nejlepsi z Parallel stejny s random, vyleze random
            stats = sorted(stats, key=itemgetter(0, 1, 2, 3, 4))
            criterionBests[criterion] = stats[-1]

        print("\\subsubsection{TODO} %%%s"%(genomeType))
        print("\\begin{table}[t]\n\\begin{center}")
        print("\\begin{tabular}{| l | c | c | c | c | c | c |}\n\\hline")
        print("Kritérium & pMut & maximum & medián & std. odchylka & průměr & minimum \\\\ \\hline")

        for criterion, i in criterionBests.iteritems():
            print("%s & %s & $%g$ & $%g$ & $%g$ & $%g$ & $%g$ \\\\ \\hline" %(criterion, str(i[5]), i[0], i[1], i[2] - i[3], i[3], i[4]) )
        print("\\end{tabular}\n\\caption{TODO} %%%s\n\\label{tab%sParallelRadnomParameters}\n\\end{center}\n\\end{table}" % (genomeType, genomeType))
        print("\n")


def experiment2():
    algorithms = ["Ga", "UMDA", "BMDA", "Random"]

    gaParams = {} #FIXME jine
    gaParams[CriterionFunction.BENT_AND_MOSAC] = (0.1, 0.2)
    gaParams[CriterionFunction.LP_MAX] = (0.0, 0.16)
    gaParams[CriterionFunction.DP_MAX] = (0.0, 0.2)
    gaParams[CriterionFunction.SAC] = (0.1, 0.2)
    gaParams[CriterionFunction.BF] = (0.1, 0.2)
    gaParams[CriterionFunction.POLYNOMIAL_DEGREE] = (0, 0)

    algoritmResults = {} # alrithm -> {} s criterion
    for alg in algorithms:
        algoritmResults[alg] = {}

    for criterion in allCriterions:
        genome = Genome(ChromozomeType.BINARY, criterion, 4, 4)
        results = {}

        for algorithm in algorithms:
            if algorithm == "Ga":
                func = lambda x: Searching(algorithm, genome, popSize, generations, gaParams[criterion][0], gaParams[criterion][1], False)
            elif algorithm == "Random":
                func = lambda x: Searching(algorithm, genome, popSize, generations)
            elif algorithm == "UMDA":
                func = lambda x: Searching("Eda", genome, popSize, generations, False)
            elif algorithm == "BMDA":
                func = lambda x: Searching("Eda", genome, popSize, generations, True)
            else:
                assert False

            list = map(func, xrange(runsCount))
            result = runs(list)
            results[algorithm] = result
            algoritmResults[algorithm][criterion] = result

        # pro kazde kriterium boxplot
        algorithmsComparisonBoxPlotUnsorted("edaVsGa", criterion, results, "Algoritmus")

    # pro kazdy algoritmus tabulka
    for algorithm in algorithms:
        print('\n')
        print("\\begin{table}[tb]\n\\begin{center}")
        print("\\begin{tabular}{| l | c | c | c | c | c |}\n\\hline")
        print("Kritérium & maximum & medián & std. odchylka & průměr & minimum \\\\ \\hline")
        for criterion, data in algoritmResults[algorithm].iteritems():
            i = runsStats(data)
            print("%s & $%g$ & $%g$ & $%g$ & $%g$ & $%g$ \\\\ \\hline" %(criterion, i[0], i[1], i[2] - i[3], i[3], i[4]) )
        print("\\end{tabular}\n\\caption{TODO %s}\n\\label{tab%sGaVsEDAComparison}\n\\end{center}\n\\end{table}" % (algorithm, algorithm))

def crossAndMutationBoxPlot(results, criterion):
    sortedResultsKeys = sorted(results, reverse=True)
    figure()
    ylabel("Parametry (pMut, pCross)")
    xlabel(criterion)
    yticks(arange(len(results)), map(lambda x: "(%.2g,%.1f)"%(x[0], x[1]), sortedResultsKeys))
    boxplot(map(lambda x: results[x], sortedResultsKeys), vert=0)
    boxplotFileName = "boxplot%s"%(criterion)
    savefig("%s/%s.%s"%(OUTPUT_DIR, boxplotFileName, OUTPUT_IMAGE_FORMAT))

def experiment1():
    for criterion in allCriterions:
        results = {}
        genome = Genome(ChromozomeType.PERMUTATION, criterion, True, 4, 4)

        for pMut in [x/25. for x in xrange(5)] :
            for pCross in [x/25. for x in xrange(5)]:
                results[(pMut, pCross)] = runs(map(lambda x: Searching("Ga", genome, popSize, generations, pMut, pCross, False), xrange(runsCount)))

        #nakresleni grafu...
        crossAndMutationBoxPlot(results, criterion)

        stats = []
        for (pMut, pCross), values in results.iteritems():
            stats.append(runsStats(values) + [pCross, pMut])

        #key of max value... serazeni podle klice [max, median, mean+dev, mean, min, pCross, pMut]
        stats = sorted(stats, key=itemgetter(5,6), reverse=True)
        stats = sorted(stats, key=itemgetter(0, 1, 2, 3, 4)) # garantovana stabilita

        #vytisknout vsechny statistiky...
        print("\\subsubsection{TODO} %%%s"%(criterion))
        print("\\begin{table}[t]\n\\begin{center}")
        print("\\begin{tabular}{| l | c | c | c | c | c |}\n\\hline")
        print("Parametry & maximum & medián & std. odchylka & průměr & minimum \\\\ \\hline")

        for i in stats:
            print("(%g, %g) & $%g$ & $%g$ & $%g$ & $%g$ & $%g$ \\\\ \\hline" %(i[6], i[5], i[0], i[1], i[2] - i[3], i[3], i[4]) )
        print("\\end{tabular}\n\\caption{TODO} %%%s\n\\label{tab%sParameters}\n\\end{center}\n\\end{table}" % (criterion, criterion))

        print("\\insertimage[0.88\\textwidth]{%s}{TODO} %%%s"%(boxplotFileName, criterion))

        bestLine = stats[-1]
        best = (bestLine[6], bestLine[5])

        picleSaveTo(results, "%s/%sExp1GA.picle" % (OUTPUT_DIR, criterion))

        #nahodne prohledavani, zavolat funkci algorithmComparisonboxplot?
        resultOfRandomSearch = runs(map(lambda x: Searching("Random", genome, popSize, generations), xrange(runsCount)))
        figure()
        xlabel("Algoritmus")
        ylabel(criterion)
        xticks(arange(2,), ["GA", "Random"])
        boxplot([results[best], resultOfRandomSearch])
        comparisonFileName = "comparison%ssmall"%(criterion)
        savefig("%s/%s.%s"%(OUTPUT_DIR, comparisonFileName, OUTPUT_IMAGE_FORMAT))
        # TODO posunout
        randomStats = runsStats(resultOfRandomSearch)
        print ("náhodné prohledávání: maximum %g medián %g odchylka %g průměr %g minimum %g" %
               (randomStats[0], randomStats[1], randomStats[2] - randomStats[3], randomStats[3], randomStats[4]))
        print("\\insertimage[0.88\\textwidth]{%s}{TODO} %%%s"%(comparisonFileName, criterion))

        picleSaveTo(randomStats, "%s/%sExp1RandomSmall.picle" % (OUTPUT_DIR, criterion))
        print("\n")

def runs(searchingList):
    Searching.parallelSearching(searchingList, TerminationCondition.GENERATION)
    #spojeni vysledku
    results = numpy.empty(0, dtype=numpy.float32)
    for searching in searchingList:
        results = numpy.append(results, searching.statistics().bestPopulationScores)
    map(lambda s: s.close(), searchingList);
    return results

def runsWithStrings(searchingList):
    Searching.parallelSearching(searchingList, TerminationCondition.GENERATION)
    #spojeni vysledku
    results = numpy.empty(0, dtype=numpy.float32)
    strings = []
    for searching in searchingList:
        results = numpy.append(results, searching.statistics().bestPopulationScores)
        strings += searching.bestPopulationStrings()
        print searching.statistics().bestPopulationOutputs
    map(lambda s: s.close(), searchingList);
    return results, strings

def multicriterialRuns(searchingList, criterions):
    Searching.parallelSearching(searchingList, TerminationCondition.GENERATION)
    criterionsValues = {}
    solutionValues = numpy.empty([0, CRITERIONS_COUNT], dtype=numpy.float32)
    solutionStrings = []

    for criterion in criterions:
        criterionsValues[criterion] = numpy.empty(0, dtype=numpy.float32);
    for searching in searchingList:
        stats = searching.statistics()
        runCriterionsValues = stats.bestPopulationCriterionsValues.T
        for criterion in criterions:
            criterionsValues[criterion] = numpy.append(criterionsValues[criterion], runCriterionsValues[criterion.ordinal])
        solutionValues = numpy.concatenate((solutionValues, stats.bestPopulationCriterionsValues), axis = 0)
        solutionStrings += searching.bestPopulationStrings()

    map(lambda s: s.close(), searchingList);
    return criterionsValues, solutionValues, solutionStrings

def runsStats(values):
    mean = numpy.mean(values)
    return [numpy.nanmax(values), numpy.median(values), mean + numpy.std(values), mean, numpy.nanmin(values)]

def algorithmsComparisonBoxPlotUnsorted(exp, yLabel, data, xLabel):
    algorithmsComparisonBoxPlot(exp, yLabel, data.values(), data.keys(), xLabel)

def algorithmsComparisonBoxPlotSorted(exp, yLabel, data, xLabel, reversed=False):
    xData = sorted(data, reverse=reversed)
    yData = map(lambda x: data[x], xData)
    algorithmsComparisonBoxPlot(exp, yLabel, yData, xData, xLabel)

def algorithmsComparisonBoxPlot(exp, yLabel, ydata, xdata, xLabel):
    figure()
    xlabel(xLabel)
    ylabel(yLabel)
    xticks(arange(len(xdata),), xdata)
    boxplot(ydata)
    comparisonFileName = "%sComparison%s"%(exp, yLabel)
    savefig("%s/%s.%s"%(OUTPUT_DIR, comparisonFileName, OUTPUT_IMAGE_FORMAT))
    print("\\insertimage[0.88\\textwidth]{%s}{TODO} %%%s"%(comparisonFileName, yLabel))

def weakDominatingOver(solution1, solution2, criterions):
    for criterion in criterions:
        if (solution1[criterion.ordinal] < solution2[criterion.ordinal]):
            return False
    return True

def strengthDominatingOver(solution1, solution2, criterions):
    isOneGreater = False
    for criterion in criterions:
        if (solution1[criterion.ordinal] < solution2[criterion.ordinal]):
            return False
        if (solution1[criterion.ordinal] > solution2[criterion.ordinal]):
            isOneGreater = True
    return isOneGreater

def nondominance(array, criterions):
    result = []
    toDel = []
    for i in xrange(len(array)):
        for i, d in enumerate(toDel):
            del result[d-i]
        toDel = []
        resultSize = len(result)
        for j in xrange(resultSize+1):
            if j == resultSize:
                #na konci testu se vsemi z nondominance, g1 by melo patrit mezi ne (vzajemne s nimy nedominuje)
                result.append(array[i])
                break
            else:
                if strengthDominatingOver(result[j], array[i], criterions):
                    break #!continue vnejsiho cyklu !
                elif strengthDominatingOver(array[i], result[j], criterions):
                    toDel.append(j)
    return result

def main(argv):
    if (len(argv) != 2):
        print("Usage: python experiments.py n\nWhere n is number of experiment")
        print("1 - deteriming of best probabilities of mutation and crossover")
        print("2 - GA vs. EDA comparison")
        print("3 - ParallelRandom parameters")
        print("4 - Vega and Spea parameters")
        print("5 - Greater s-boxes")
        print("6 - Symbolical regresion")
        return 1
    spring()
    os.mkdir(OUTPUT_DIR)
    eval("experiment%d()" % (int(argv[1])))
    return 0

if __name__ == '__main__':
    errorHandling()
    exitStatus = main(sys.argv)
    print("Exited with status: %d" % (exitStatus))
