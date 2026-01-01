'''
      Brno University of Technology
      Faculty of Information Technology

      MSc Thesis       2009/2010

      Design of S-Boxes Using Genetic Algorithms

      Created on Jan 19, 2010

      @author: Bedrich Hovorka
'''
import ctypes, numpy, os, pickle, random

# Load libsboxevolution.so C++ library with multiple fallback locations
def _load_sbox_library():
    """Load libsboxevolution.so from environment variable or default locations."""
    library_name = "libsboxevolution.so"

    # Priority 1: SBOX_LIBRARY_PATH environment variable
    env_path = os.environ.get("SBOX_LIBRARY_PATH")
    if env_path:
        library_path = os.path.join(env_path, library_name)
        if os.path.exists(library_path):
            return ctypes.CDLL(library_path)
        else:
            raise FileNotFoundError(
                f"Library not found at SBOX_LIBRARY_PATH: {library_path}"
            )

    # Priority 2: Same directory as this script (original behavior)
    script_dir = os.path.dirname(os.path.realpath(__file__))
    library_path = os.path.join(script_dir, library_name)
    if os.path.exists(library_path):
        return ctypes.CDLL(library_path)

    # Priority 3: Current working directory
    cwd_path = os.path.join(os.getcwd(), library_name)
    if os.path.exists(cwd_path):
        return ctypes.CDLL(cwd_path)

    # Priority 4: System library path (via ctypes default search)
    try:
        return ctypes.CDLL(library_name)
    except OSError:
        raise FileNotFoundError(
            f"Could not find {library_name}. Tried:\n"
            f"  1. SBOX_LIBRARY_PATH: {env_path or 'not set'}\n"
            f"  2. Script directory: {script_dir}\n"
            f"  3. Current directory: {os.getcwd()}\n"
            f"  4. System library path\n"
            f"Set SBOX_LIBRARY_PATH environment variable to the directory containing {library_name}"
        )

libsboxevolution = _load_sbox_library()
libsboxevolution.simpleReportSearching.restype = ctypes.py_object
libsboxevolution.bestPopulationStringsSearching.restype = ctypes.py_object

# python/ceckove enumy
class Enum(object):
    def __init__(self, enumerate, name, ordinal):
        self.enumerate = enumerate
        self.name = name
        self.ordinal = ordinal

    def __repr__(self):
        return self.name

class Enumerate(object):

    def __init__(self, names):
        self.values = []
        for ordinal, name in enumerate(names.split()):
            newEnum = Enum(self, name, ordinal)
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
    with open(fileName, "rb") as file:  # Binary mode required in Python 3
        obj = pickle.load(file)
    return obj

def picleSaveTo(obj, fileName):
    with open(fileName, "wb") as file:  # Binary mode required in Python 3
        pickle.dump(obj, file)

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
            for i in range(len(array)):
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

        self.algorithm = algorithm
        self.genome = genome
        self.args = args

    def __getattr__(self, name):
        if name in ["__getstate__", "__setstate__", "__getinitargs__"]:
            raise AttributeError
        return lambda *args: libsboxevolution.__getattr__(name+"Searching")(self.delegat, *args)

    def simpleReport(self):
        print(self)
        # NOTE: simpleReportSearching() disabled due to architectural limitation
        # The C++ function returns PyObject* (Python string), but when loaded via
        # ctypes.CDLL, the library cannot safely call Python C API functions like
        # PyUnicode_FromStringAndSize(). This would require converting to a proper
        # Python extension module (not ctypes). Detailed stats are available via statistics().
        # ptr = libsboxevolution.simpleReportSearching(self.delegat)
        # print(ptr)

    def bestPopulationStrings(self):
        # NOTE: bestPopulationStringsSearching() disabled due to architectural limitation
        # Same issue as simpleReportSearching() - cannot return PyObject* from ctypes library.
        # Use statistics().bestPopulationOutputs to get S-box lookup tables as integers instead.
        # return libsboxevolution.bestPopulationStringsSearching(self.delegat)
        return []  # Return empty list to maintain compatibility

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
        for i in range(len(array)):
            array[i] = list[i].delegat
            seedArray[i] = cls.randomSeed()

        libsboxevolution.parallelProcessSearching(len(list), array, terminator.ordinal, seedArray)

    @classmethod
    def parallelSimpleSearching(cls, list, terminator=TerminationCondition.GENERATION):
        cls.parallelSearching(list, terminator)
        for s in list:
            Searching.simpleReport(s)
        for s in list:
            s.close()

    def __getstate__(self):
        """Return state values to be pickled."""
        return (self.algorithm, self.genome, self.args)

    def __setstate__(self, state):
        """Restore state from the unpickled state values."""
        algorithm, genome, args = state
        self.__init__(algorithm, genome, *args)

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

        self.type = type
        self.criterionFunction = criterionFunction
        self.args = args

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

    def __getstate__(self):
        """Return state values to be pickled."""
        return (self.type, self.criterionFunction, self.args)

    def __setstate__(self, state):
        """Restore state from the unpickled state values."""
        type, criterionFunction, args = state
        self.__init__(type, criterionFunction, *args)

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

        self.type = type
        self.expectedOutputs = expectedOutputs
        self.args = args

    def __getstate__(self):
        """Return state values to be pickled."""
        return (self.type, self.expectedOutputs, self.args)

    def __setstate__(self, state):
        """Restore state from the unpickled state values."""
        type, expectedOutputs, args = state
        self.__init__(type, expectedOutputs, *args)

def errorHandling():
    libsboxevolution.initialization()
