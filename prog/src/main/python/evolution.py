'''
      Brno University of Technology
      Faculty of Information Technology

      MSc Thesis       2009/2010

      Design of S-Boxes Using Genetic Algorithms

      Created on Jan 19, 2010

      @author: Bedrich Hovorka
'''
import ctypes, numpy, os, pickle, random

# nacteni dynamicke knihovny napsane v C/C++, umistene ve stejnem adresari jako tento skript
path = os.path.dirname(os.path.realpath(__file__))
libsboxevolution = ctypes.CDLL("%s/libsboxevolution.so"%path)
libsboxevolution.simpleReportSearching.restype = ctypes.py_object
libsboxevolution.bestPopulationStringsSearching.restype = ctypes.py_object

# python/ceckove enumy
class Enumerate(object):
    class Enum(object):
        def __init__(self, enumerate, name, ordinal):
            self.enumerate = enumerate
            self.name = name
            self.ordinal = ordinal

        def __repr__(self):
            return self.name

    def __init__(self, names):
        self.values = []
        for ordinal, name in enumerate(names.split()):
            newEnum = self.Enum(self, name, ordinal)
            setattr(self, name, newEnum)
            self.values.append(newEnum)

# jednotlive enumy sestavene podle poradi v C
ChromozomeType = Enumerate("BINARY PERMUTATION CGP SOFTWARE_IMPL")
# LANGRANGE is not implemented
CriterionFunction = Enumerate("BENT_AND_MOSAC LP_MAX DP_MAX BIJECTIVE_SCORE SAC BF POLYNOMIAL_DEGREE MINIMAL_REALISATION LAGRANGE NONE_FITNESS")
TerminationCondition = Enumerate("GENERATION CONVERGENCE POPCONVERGENCE")

CRITERIONS_COUNT = 8

# zastupuje Ceckovu strukturu, pomoci ktere se predava ukazatel na objekt algoritmu
class TSearching(ctypes.Structure):
    _fields_ = [("algorithm", ctypes.c_void_p)]

class TGenome(ctypes.Structure):
    _fields_ = [("ptr", ctypes.c_void_p)]

# zastupuje Ceckovu strukturu, pomoci ktere se predavaji statistiky behu
class TStatistics(ctypes.Structure):
    _fields_ = [
                ("online", ctypes.c_float), #average of all scores
                ("offlineMax", ctypes.c_float), # average of the maximum scores
                ("offlineMin", ctypes.c_float), #average of the minimum scores
                # bestPopulationScores - scores from the current population.
                ("maxEver", ctypes.c_float), # maximum score ever encountered
                ("minEver", ctypes.c_float), # minimum score ever encountered
                ("generation", ctypes.c_int), # current generation number
                ("convergence", ctypes.c_float), # current convergence. Here convergence means the ratio of the nth previous best-of-generation to the current best-of-generation.
                ("selections", ctypes.c_int), # number of selections that have occurred since initialization
                ("crossovers", ctypes.c_int), # number of crossovers that have occurred since initialization
                ("mutations", ctypes.c_int), # number of mutations that have occurred since initialization
                ("replacements", ctypes.c_int), # number of replacements that have occurred since initialization
                ("nBestGenomes", ctypes.c_int)
               ]

def pickleLoadFrom(fileName):
    file = open(fileName, "r")
    obj = pickle.load(file)
    file.close()
    return obj

def picleSaveTo(obj, fileName):
    file = open(fileName, "w")
    pickle.dump(obj, file)
    file.close()

# vytvoreni a manipulace s procesem evoluce
class Searching:
    def __init__(self, algorithm, genome, *args):
        specificRepr = ''
        func = libsboxevolution.__getattr__("new"+algorithm+"Searching")
        if algorithm == "Ga" or algorithm == "Vega" or algorithm == "Spea":
            specificRepr += "probability of mutation: %f probability of crossover: %f" %(args[2], args[3])

        if algorithm == "Ga":
            func.argtypes = [TGenome, ctypes.c_int, ctypes.c_int, ctypes.c_float, ctypes.c_float, ctypes.c_int]
        elif algorithm == "ParallelRandom":
            func.argtypes = [TGenome, ctypes.c_int, ctypes.c_int, ctypes.c_float]
        elif algorithm == "Vega" or algorithm == "Spea":
            criterions = args[-1]
            TArray = ctypes.c_int * len(criterions)
            array = TArray()
            for i in xrange(len(array)):
                array[i] = criterions[i].ordinal
            args = args[:-1] + (len(array), array)
            func.argtypes = [TGenome, ctypes.c_int, ctypes.c_int, ctypes.c_float, ctypes.c_float, ctypes.c_int, TArray]
            specificRepr += '\ncriterions: %s' % (criterions)
        elif algorithm == "Eda":
            func.argtypes = [TGenome, ctypes.c_int, ctypes.c_int, ctypes.c_int]
        elif algorithm == "Random":
            func.argtypes = [TGenome, ctypes.c_int, ctypes.c_int]

        self.popSize = args[0]
        nGen = args[1]
        self.outputsCount = genome.outputsCount
        self.inputsCount = genome.inputsCount

        #print hex(genome.delegat.ptr)
        func.restype = TSearching
        self.delegat = func(genome.delegat, *args)
        self.repr = '%s searching with population size: %d and generation count: %d\ngenome: %s\n%s' % (algorithm, self.popSize, nGen, genome, specificRepr)

    def __getattr__(self, name):
        return lambda *args: libsboxevolution.__getattr__(name+"Searching")(self.delegat, *args)

    def simpleReport(self):
        print(self)
        ptr = libsboxevolution.simpleReportSearching(self.delegat)
        print(ptr)

    def bestPopulationStrings(self):
        return libsboxevolution.bestPopulationStringsSearching(self.delegat)

    def simpleEvolveAndClose(self, terminator=TerminationCondition.GENERATION):
        self.simpleEvolve(terminator)
        self.close()

    def simpleEvolve(self, terminator=TerminationCondition.GENERATION):
        self.process(terminator.ordinal, self.randomSeed())
        self.simpleReport()

    def statistics(self):
        libsboxevolution.getStatisticsSearching.restype = TStatistics
        libsboxevolution.getStatisticsSearching.argtypes = [TSearching,
                                                            numpy.ctypeslib.ndpointer(dtype = numpy.float32),
                                                            numpy.ctypeslib.ndpointer(dtype = numpy.float32),
                                                            numpy.ctypeslib.ndpointer(dtype = ctypes.c_int)]

        bestScores = numpy.empty(self.popSize, dtype=numpy.float32)
        bestPopulationCriterionsValues = numpy.empty([self.popSize, CRITERIONS_COUNT], dtype=numpy.float32)
        bestPopulationOutputs = numpy.empty([self.popSize, 2**self.inputsCount], dtype=ctypes.c_int)

        stats = libsboxevolution.getStatisticsSearching(self.delegat, bestScores, bestPopulationCriterionsValues, bestPopulationOutputs)
        stats.bestPopulationScores = bestScores[0:stats.nBestGenomes]
        stats.bestPopulationCriterionsValues = bestPopulationCriterionsValues[0:stats.nBestGenomes]
        stats.bestPopulationOutputs = bestPopulationOutputs[0:stats.nBestGenomes]
        return stats

    def __str__(self):
        return self.repr

    @classmethod
    def randomSeed(self):
        return random.randint(0, 2**32-1)

    @classmethod
    def parallelSearching(cls, list, terminator=TerminationCondition.GENERATION):
        TArray = TSearching * len(list)
        TSeedArray = ctypes.c_uint32 * len(list)
        libsboxevolution.parallelProcessSearching.argtypes = [ctypes.c_int, TArray, ctypes.c_int, TSeedArray]

        array = TArray()
        seedArray = TSeedArray()
        for i in xrange(len(array)):
            array[i] = list[i].delegat
            seedArray[i] = cls.randomSeed()

        libsboxevolution.parallelProcessSearching(len(list), array, terminator.ordinal, seedArray)

    @classmethod
    def parallelSimpleSearching(cls, list, terminator=TerminationCondition.GENERATION):
        cls.parallelSearching(list, terminator);
        map(Searching.simpleReport, list);
        map(lambda s: s.close(), list);

# tato trida pomaha urcit, jaky typ reprezentace a jakou hodnotici funkci (pri jednokriterialnim) reprezentace se pouzije
class Genome:
    def __init__(self, type, criterionFunction, *args):
        l = len(args)
        libsboxevolution.createGenome.argtypes = [ctypes.c_uint, ctypes.c_uint, ctypes.c_int] + [ctypes.c_int]*l
        libsboxevolution.createGenome.restype = TGenome
        self.delegat = libsboxevolution.createGenome(type.ordinal, criterionFunction.ordinal, l, *args)

        #print "init", hex(self.delegat.ptr)
        self.setInputsAndOutputs(type, *args)

        self.repr = '%s %s' % (type, args)
        if criterionFunction is not CriterionFunction.NONE_FITNESS:
            self.repr += ' criterion: %s' % criterionFunction

    def __str__(self):
        return self.repr

    def setInputsAndOutputs(self, type, *args):
        if type == ChromozomeType.SOFTWARE_IMPL:
            self.outputsCount = args[0]*4
            self.inputsCount = args[0]*4
        elif type == ChromozomeType.PERMUTATION:
            self.inputsCount = args[1]
            self.outputsCount = args[2]
        else:
            self.inputsCount = args[0]
            self.outputsCount = args[1]

class SymbolicalRegresionGenome(Genome):
    def __init__(self, type, expectedOutputs, *args):
        self.setInputsAndOutputs(type, *args)
        assert len(expectedOutputs) == 2**self.inputsCount;
        l = len(args)
        libsboxevolution.createSymbolicalRegresionGenome.argtypes = [ctypes.c_uint, numpy.ctypeslib.ndpointer(dtype = ctypes.c_int), ctypes.c_int] + [ctypes.c_int]*l
        libsboxevolution.createSymbolicalRegresionGenome.restype = TGenome
        self.delegat = libsboxevolution.createSymbolicalRegresionGenome(type.ordinal, numpy.asarray(expectedOutputs, dtype=ctypes.c_int), l, *args)

        self.repr = '%s %s' % (type, args)
        self.repr += ' symbolical regresion '

def errorHandling():
    libsboxevolution.initialization()
