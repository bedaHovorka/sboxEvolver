# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

**S-box Evolver** - A research project implementing evolutionary algorithms for the design and optimization of cryptographic S-boxes (Substitution boxes).

This is a Master's thesis project from Brno University of Technology, Faculty of Information Technology (2009/2010), authored by Bedrich Hovorka. The project explores using genetic algorithms and other evolutionary computation methods to search for optimal S-boxes based on multiple cryptographic criteria.

**Recent Updates (2025)**:
- **January 2025**: Migrated from Python 2.7 to Python 3.14
  - Modernized Python syntax (print functions, range, dict methods)
  - Added uv package manager for dependency management
  - Updated dependencies (numpy 1.26.4, matplotlib 3.9+, pytest 8.3+)
  - Environment-based library loading via SBOX_LIBRARY_PATH
  - Optimized Docker multi-stage build (removed build tools from runtime)
  - **Debian Package Distribution**: Docker build now creates `libsboxevolution_2.0.0_amd64.deb` package that installs library to `/usr/lib/` following FHS standards
  - **Known Limitation**: Two Python C API functions (`simpleReportSearching()` and `bestPopulationStringsSearching()`) are disabled due to architectural constraints with ctypes-loaded libraries. Use `statistics().bestPopulationOutputs` for S-box data instead.
- **March 2026**: Comprehensive test suite
  - C++ upgraded from C++98 to **C++11** (`-std=gnu++11`)
  - **Catch2 v2** (single-header) for C++ unit tests: 55 test cases, 1035 assertions
  - **pytest** with hypothesis, pytest-benchmark, pytest-cov, pytest-xdist for Python: 26 tests
  - **Valgrind** memory leak checking gate in Docker build
  - Docker build gates: all C++ tests, Python tests, and Valgrind must pass
  - On-demand C++ and Python benchmarks (not in CI)
- Restructured with improved organization, Docker support, unit tests, and enhanced documentation while preserving the original research implementation

## Project Structure

```
sboxEvolver/
├── prog/                           # Source code (Python + C++)
│   ├── Dockerfile                  # Multi-stage build container
│   ├── makefile                    # Build configuration
│   └── src/
│       ├── main/                   # Source files (reorganized 2025)
│       │   ├── cpp/                # C++ implementation
│       │   │   ├── main.cpp        # C++ core implementation (GAlib integration)
│       │   │   ├── main.h          # Main header
│       │   │   ├── common.h        # Common definitions
│       │   │   ├── cgp.cpp/h       # Cartesian Genetic Programming representation
│       │   │   ├── softwareSbox.cpp/h  # Software implementation of S-boxes
│       │   │   ├── eda.cpp/h       # Estimation of Distribution Algorithms
│       │   │   ├── criterions.cpp/h    # Cryptographic fitness functions
│       │   │   ├── multicriterial.cpp/h # VEGA and SPEA algorithms
│       │   │   ├── boxes.h         # S-box lookup tables
│       │   │   └── combination.h   # Combinatorial utilities
│       │   └── python/             # Python interface
│       │       ├── evolution.py    # Python wrapper for evolutionary algorithms
│       │       └── experiments.py  # Experimental framework with 6 experiments
│       └── test/                   # Unit tests (added 2025)
│           ├── cpp/                # C++ tests (Catch2 v2)
│           │   ├── catch.hpp           # Catch2 v2.13.10 single header
│           │   ├── test_main.cpp       # Catch2 entry point
│           │   ├── test_criterions.cpp # Cryptographic criterion tests
│           │   ├── test_boxes.cpp      # KAT + genome class tests
│           │   ├── test_combination.cpp # Combinatorial utility tests
│           │   ├── test_softwareSbox.cpp # Software S-box tests
│           │   ├── test_cgp.cpp        # CGP genome tests
│           │   ├── test_eda.cpp        # EDA model tests
│           │   ├── test_multicriterial.cpp # Multi-objective tests
│           │   ├── bench_main.cpp      # Benchmark entry point
│           │   └── bench_criterions.cpp # Performance benchmarks
│           └── python/
│               ├── conftest.py          # Shared pytest fixtures
│               ├── test_routines.py     # Core algorithm and routine tests
│               ├── test_experiments.py  # Experiment framework tests
│               ├── test_hypothesis.py   # Property-based tests (hypothesis)
│               ├── test_integration.py  # Multi-objective + parallel integration
│               └── test_benchmark.py    # Performance benchmarks (on-demand)
├── text/                           # LaTeX thesis document
│   ├── Dockerfile                  # LaTeX build container
│   ├── *.utf8.tex                  # Thesis chapters
│   ├── diplomka.bib                # Bibliography
│   ├── img/                        # Figures and diagrams
│   └── Makefile                    # LaTeX build
├── artifacts/                      # Build outputs (gitignored)
│   ├── prog/
│   │   ├── libsboxevolution.so           # Generated C++ shared library (for native builds)
│   │   └── libsboxevolution_2.0.0_amd64.deb  # Debian package (for system installation)
│   └── text/
│       └── diplomka.pdf           # Generated thesis PDF
├── outputs/                        # Experiment results and logs
├── docker-compose.yml              # Docker orchestration
├── galib247bh100329.tar.bz2       # GAlib library (genetic algorithm framework)
├── DIPxhovor07final.pdf           # Final thesis PDF
└── README.md                      # Project documentation

```

## Architecture

### Hybrid Python/C++ Design

The project uses a **two-tier architecture**:

1. **C++ Core** (`prog/src/main/cpp/*.cpp/h`): High-performance evolutionary algorithms using GAlib
2. **Python Interface** (`prog/src/main/python/evolution.py`): Wraps C++ via ctypes for easy experimentation

### Key Components

#### 1. Genome Representations (Chromosome Types)

- **BINARY**: Binary string representation (traditional GA)
- **PERMUTATION**: Permutation-based encoding (bijective S-boxes)
- **CGP**: Cartesian Genetic Programming (evolved circuits)
- **SOFTWARE_IMPL**: Software-optimized lookup table implementation

#### 2. Evolutionary Algorithms

- **Ga**: Standard Genetic Algorithm
- **Random**: Random search baseline
- **ParallelRandom**: Mutate-best-parallel strategy
- **Eda**: Estimation of Distribution Algorithm (UMDA/BMDA variants)
- **Vega**: Vector Evaluated Genetic Algorithm (multi-objective)
- **Spea**: Strength Pareto Evolutionary Algorithm (multi-objective)

#### 3. Cryptographic Fitness Functions (Criterions)

The S-boxes are evaluated against multiple cryptographic quality metrics:

- **BENT_AND_MOSAC**: Combined bentness and maximum-order SAC
- **LP_MAX**: Maximum Linear Probability (resistance to linear cryptanalysis)
- **DP_MAX**: Maximum Differential Probability (resistance to differential cryptanalysis)
- **SAC**: Strict Avalanche Criterion (output bit change probability)
- **BF**: Balance and Completeness Factor
- **POLYNOMIAL_DEGREE**: Nonlinearity measure
- **BIJECTIVE_SCORE**: Bijectivity constraint satisfaction
- **MINIMAL_REALISATION**: Circuit complexity

#### 4. Python-C++ Bridge (`evolution.py`)

Uses `ctypes` to load the compiled shared library:

```python
libsboxevolution = ctypes.CDLL("libsboxevolution.so")

# Create genome
genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)

# Create search algorithm
searching = Searching("Ga", genome, popSize=1000, nGen=50, pMut=0.1, pCross=0.9)

# Run evolution
searching.simpleEvolve(TerminationCondition.GENERATION)
```

## Development Commands

### Docker Build (Recommended)

The project includes Docker support for reproducible builds without manual dependency setup:

```bash
# Build both C++ library and thesis PDF
docker-compose up --build

# Build individually
docker-compose build prog   # Builds libsboxevolution.so
docker-compose build text   # Builds diplomka.pdf

# Artifacts appear in ./artifacts/
# - artifacts/prog/libsboxevolution.so           (raw library for native builds)
# - artifacts/prog/libsboxevolution_2.0.0_amd64.deb (Debian package for system installation)
# - artifacts/text/diplomka.pdf
```

**Docker Architecture:**
- **prog service**: Multi-stage optimized build
  - Stage 1 (`galib-builder`): Build GAlib 2.4.7
  - Stage 2 (`sbox-builder`): Build libsboxevolution.so (C++11) and create Debian package
  - Stage 3 (`deps-builder`): Build Python 3.14 dependencies (numpy compilation with gcc/g++)
  - Stage 4 (`python-base`): Clean Python 3.14 runtime base (installs Debian package to /usr/lib/, no build tools)
  - Stage 5a (`valgrind-tester`): Valgrind memory leak check (must pass)
  - Stage 5b (`cpp-tester`): C++ unit tests with Catch2 v2 (55 test cases, must pass)
  - Stage 5c (`tester`): Python tests with pytest (26 tests, must pass)
  - Stage 6 (`runtime`): Production runtime (depends on all test stages passing)
- **text service**: Single-stage LaTeX compilation with Czech language tools
- **No inter-service dependencies**: Services build in parallel

**Build gates — all must pass for successful build:**
- Valgrind: 0 memory errors
- C++ tests: 55 test cases, 1035 assertions (Catch2 v2)
- Python tests: 26 tests (pytest + hypothesis + assertpy)

**Advantages:**
- No manual dependency installation (GAlib extraction, LaTeX tools, Czech typography packages)
- Reproducible builds across platforms
- Isolated build environments with minimal runtime image size
- Automatic makefile patching for modern compilers (g++-4.4 → g++, k8-sse3 → native)
- Python 3.14 support with modern pytest 8.3+
- Build tools (gcc/g++) only in build stages, not in runtime (reduces image size ~200MB)
- Resolves all platform-specific limitations mentioned in this document

**Debian Package:**

The Docker build creates a proper Debian package (`libsboxevolution_2.0.0_amd64.deb`) that can be installed on Debian-based systems:

```bash
# After Docker build, the package is available in artifacts/
ls artifacts/prog/libsboxevolution_2.0.0_amd64.deb

# Install on Debian/Ubuntu system
sudo dpkg -i artifacts/prog/libsboxevolution_2.0.0_amd64.deb

# Or install dependencies automatically
sudo apt-get install -y libgomp1
sudo apt-get install ./artifacts/prog/libsboxevolution_2.0.0_amd64.deb

# Verify installation
dpkg -l | grep libsboxevolution
ldconfig -p | grep libsboxevolution  # Should show /usr/lib/libsboxevolution.so

# Uninstall
sudo dpkg -r libsboxevolution
```

**Package Details:**
- **Location**: Library installs to `/usr/lib/libsboxevolution.so` (standard FHS path)
- **Dependencies**: libgomp1 (OpenMP), libc6 (>= 2.28)
- **Architecture**: amd64 (x86_64) only
- **Debug symbols**: Included (useful for debugging with gdb)
- **Library loading**: Python wrapper (`evolution.py`) will automatically find the library via system paths

### Native Build (Manual)

**⚠️ Docker is strongly recommended to avoid these manual steps.**

For development without Docker:

```bash
cd prog/

# Extract and compile GAlib (one-time setup)
tar -xjf ../galib247bh100329.tar.bz2 -C ..
cd ../galib247
make
cd ../prog

# Compile the S-box evolution library
make clean
make

# This produces libsboxevolution.so (build artifact, not in git)
```

**Dependencies:**
- `g++` with C++11 support (`-std=gnu++11`)
- OpenMP support (`-fopenmp`)
- Python 3.14+ development headers
- GAlib 2.4.7 (extracted from included tarball)

**⚠️ Manual makefile adjustments required** (automatically done by Docker):
- Line 18: `CXX = g++-4.4` → `CXX = g++` (if g++-4.4 unavailable)
- Line 27: `-march=k8-sse3` → `-march=native` (for modern CPUs)
- Python include paths: Update to your Python 3.14 installation path

**Note:** The shared library `libsboxevolution.so` is a **build artifact** generated by the build process. It is not checked into version control.

### Running Experiments

The `experiments.py` file contains 6 numbered experiments from the thesis:

**Docker (Recommended):**
```bash
# Build and run experiment 1 (default)
docker-compose up --build prog

# Run different experiment (set environment variable)
EXPERIMENT_NUM=3 docker-compose up prog
```

**Local Development (Python 3.14 + uv):**
```bash
cd prog/

# First, build C++ library with Docker (one-time setup)
docker-compose build prog

# Install Python dependencies with uv
uv sync

# Set library path to Docker-compiled artifact
export SBOX_LIBRARY_PATH=/home/beda/PycharmProjects/sboxEvolver/artifacts/prog

# Run specific experiment (1-6)
uv run python src/main/python/experiments.py <experiment_number>

# Examples:
uv run python src/main/python/experiments.py 1  # Mutation/crossover parameter tuning
uv run python src/main/python/experiments.py 2  # GA vs. EDA comparison
uv run python src/main/python/experiments.py 3  # ParallelRandom parameter search
uv run python src/main/python/experiments.py 4  # VEGA/SPEA parameter tuning
uv run python src/main/python/experiments.py 5  # Larger S-boxes (6x6, 8x8)
uv run python src/main/python/experiments.py 6  # Symbolic regression
```

**Output:**
- Creates timestamped directory: `./results<YYYYMMDDHHMMSS>/` (in `prog/` directory)
- Generates EPS plots and LaTeX tables
- Saves serialized results (`.picle` files)

### Running Tests

**All tests must pass before merging.** The Docker build enforces this — it fails if any test gate fails.

**Docker (Recommended):**

All three test suites run automatically during `docker-compose build prog`:

```bash
# Build and run all tests (C++, Python, Valgrind)
docker-compose build prog

# The build fails if any test gate fails:
# - Stage 5a: Valgrind memory leak check
# - Stage 5b: C++ unit tests (Catch2 v2, 55 test cases)
# - Stage 5c: Python tests (pytest, 26 tests)
```

**Native C++ tests (requires GAlib built locally):**

```bash
cd prog/

# Run C++ unit tests
make test

# Run C++ benchmarks (on-demand, not in CI)
make bench

# Run Valgrind memory check
make valgrind
```

**Native Python tests (requires compiled library):**

```bash
cd prog/

# Ensure library path is set
export SBOX_LIBRARY_PATH=/path/to/sboxEvolver/artifacts/prog

# Run all tests (excludes benchmarks by default via addopts)
uv run pytest src/test/python/

# Run with coverage report
uv run pytest --cov=src/main/python --cov-report=term-missing src/test/python/

# Run benchmarks only (on-demand)
uv run pytest -m benchmark --benchmark-only src/test/python/test_benchmark.py
```

**Test Suite Overview:**

| Suite | Framework | Count | What it covers |
|---|---|---|---|
| C++ unit tests | Catch2 v2.13.10 | 55 cases, 1035 assertions | criterions, boxes, combination, softwareSbox, cgp, eda, multicriterial |
| Python tests | pytest + hypothesis + assertpy | 26 tests | routines, experiments, property-based, integration |
| Python benchmarks | pytest-benchmark | 4 tests | GA, CGP, EDA, statistics (on-demand, `@pytest.mark.benchmark`) |
| C++ benchmarks | Catch2 benchmarks | 6 benchmarks | criterion functions, GA step (on-demand via `make bench`) |
| Valgrind | valgrind --leak-check=full | 1 harness | Memory leak detection |

**C++ Test Files:**
- `test_criterions.cpp` — hammingWeight, binaryDot, highestOneBit, computeLPMax, computeDPMax, isBijective, isBidirectional, bijectiveScore, computeBranchingFactor, SAC, bent, polynomial
- `test_boxes.cpp` — BinarySboxGenome, PermutationSboxGenome, KAT tests, criterion dispatch
- `test_combination.cpp` — next_combination, prev_combination enumeration
- `test_softwareSbox.cpp` — Luffa SubCrumb testBox() KAT, C API integration, random init
- `test_cgp.cpp` — CGP creation, output range, multiple criteria, symbolic regression
- `test_eda.cpp` — ContingentTable chi-squared, BMDA/UMDA via C API
- `test_multicriterial.cpp` — VEGA, SPEA via C API, criterion validation

**Python Test Files:**
- `test_routines.py` — Core algorithm and routine tests
- `test_experiments.py` — Experiment framework tests
- `test_hypothesis.py` — Property-based tests (bijective outputs, score invariants)
- `test_integration.py` — VEGA, SPEA, parallel search, deterministic seed
- `test_benchmark.py` — Performance benchmarks (on-demand only)

**Prerequisites:**
- Compiled `libsboxevolution.so` must be present
- Must run from `prog/` directory for correct library path resolution
- Python 3.14+ with numpy and matplotlib
- pytest 8.3+, assertpy, hypothesis, pytest-benchmark, pytest-cov (installed via `uv sync`)

### Thesis Compilation

**Docker Build (Recommended):**
```bash
# Build thesis PDF in Docker
docker-compose up --build text

# Output appears at: ./artifacts/text/diplomka.pdf
```

**Native Build:**
```bash
cd text/

# Compile LaTeX thesis (full build)
make

# Or clean and rebuild
make rebuild

# Clean auxiliary files only
make clean

# Remove all generated files including PDF
make mrproper
```

**Required Tools:**

LaTeX compilation requires the following tools to be installed:

1. **Core LaTeX Tools**:
   - `pdflatex` - LaTeX to PDF compiler
   - `bibtex` - Bibliography processor for .bib files

2. **Encoding Conversion**:
   - `cstocs` - Czech text encoding converter (UTF-8 → ISO-8859-2/IL2)
   - `vlna` - Czech typography tool (adds non-breaking spaces per Czech rules)

3. **Image Processing**:
   - `epstopdf` - EPS to PDF converter for plots
   - `inkscape` - SVG to PDF/EPS converter
   - `dia` - Diagram editor and export tool

4. **Optional**:
   - `gnuplot` - For regenerating plots from .gp files (if present)

**Installation (Fedora/RHEL):**
```bash
sudo dnf install texlive texlive-epstopdf inkscape dia
# Note: cstocs and vlna may need to be installed from source or Czech-specific repos
```

**Installation (Debian/Ubuntu):**
```bash
sudo apt install texlive-latex-base texlive-latex-extra texlive-bibtex-extra \
                 texlive-fonts-recommended texlive-lang-czechslovak \
                 texmaker epstopdf inkscape dia
```

**Build Process:**

The Makefile performs these steps:
1. Converts UTF-8 .tex files to ISO-8859-2 encoding (Czech standard)
2. Applies Czech typographic rules with `vlna`
3. Generates abbreviations list from `zkratky.dat`
4. Converts all EPS images to PDF
5. Runs `pdflatex` → `bibtex` → `pdflatex` × 2 (for cross-references)
6. Produces `diplomka.pdf` (thesis PDF)

**Notes:**
- The thesis is written in Czech language
- Source files use UTF-8 encoding (`.utf8.tex`)
- Output encoding is ISO-8859-2 (Latin-2)
- Three pdflatex passes are needed for correct TOC and references

## Experiment Descriptions

### Experiment 1: Parameter Tuning for GA
- **Genome**: Permutation (4x4 S-boxes)
- **Goal**: Find optimal mutation and crossover probabilities
- **Output**: Box plots comparing parameter combinations, comparison with random search

### Experiment 2: GA vs. EDA
- **Genome**: Binary representation
- **Algorithms**: GA, UMDA, BMDA, Random
- **Criteria**: All six cryptographic criteria
- **Output**: Statistical tables and box plots

### Experiment 3: ParallelRandom Optimization
- **Genomes**: CGP and SOFTWARE_IMPL
- **Goal**: Optimize mutation probability for parallel random search
- **Output**: Parameter sensitivity analysis

### Experiment 4: Multi-objective Parameter Tuning
- **Algorithms**: VEGA and SPEA
- **Goal**: Find Pareto-optimal parameter settings
- **Metric**: Count of above-average individuals (weak dominance)

### Experiment 5: Larger S-boxes
- **Dimensions**: 4x4, 6x4, 6x6, 8x8
- **Algorithms**: VEGA, SPEA on multiple representations
- **Output**: Non-dominated solution sets

### Experiment 6: Symbolic Regression
- **Goal**: Evolve S-box as algebraic expressions using CGP
- **Training**: Random permutation target

## Key Implementation Patterns

### Creating and Running a Search

```python
from evolution import *

# 1. Define genome (representation + criterion)
genome = Genome(
    ChromozomeType.PERMUTATION,  # Representation
    CriterionFunction.DP_MAX,     # Fitness function
    True,                          # Bijective constraint
    4,                             # Input bits
    4                              # Output bits
)

# 2. Create search algorithm
search = Searching(
    "Ga",                 # Algorithm type
    genome,
    popSize=1000,         # Population size
    nGen=50,              # Number of generations
    pMut=0.1,             # Mutation probability
    pCross=0.9,           # Crossover probability
    False                 # Elitism flag (Ga only)
)

# 3. Run evolution
search.simpleEvolve(TerminationCondition.GENERATION)

# 4. Get results
stats = search.statistics()
print("Best score:", stats.maxEver)
print("Best S-boxes:", search.bestPopulationStrings())

# 5. Cleanup
search.close()
```

### Multi-objective Optimization (VEGA/SPEA)

```python
# Define multiple criteria
criterions = [
    CriterionFunction.LP_MAX,
    CriterionFunction.DP_MAX,
    CriterionFunction.SAC
]

# Genome with no single criterion (multi-objective)
genome = Genome(ChromozomeType.CGP, CriterionFunction.NONE_FITNESS, 4, 4, 4, 3, 6)

# VEGA or SPEA
search = Searching("Vega", genome, 1000, 50, 0.1, 0.9, criterions)
search.simpleEvolve()

# Extract Pareto front
stats = search.statistics()
criterionsValues = stats.bestPopulationCriterionsValues  # Array of shape [popSize, 8]
```

### Parallel Execution

```python
# Create list of search instances
searches = [
    Searching("Ga", genome, 1000, 50, 0.1, 0.9, False)
    for _ in range(40)  # 40 independent runs
]

# Run in parallel (uses OpenMP in C++)
Searching.parallelSearching(searches, TerminationCondition.GENERATION)

# Collect results
for s in searches:
    print(s.statistics().maxEver)
    s.close()
```

### Statistics Structure

```python
stats = search.statistics()

# Scalar metrics
stats.online           # Average of all scores
stats.offlineMax       # Average of maximum scores
stats.maxEver          # Best score ever found
stats.generation       # Current generation
stats.convergence      # Convergence ratio

# Population arrays (length: nBestGenomes)
stats.bestPopulationScores           # Fitness values [popSize]
stats.bestPopulationCriterionsValues # All criteria [popSize, 8]
stats.bestPopulationOutputs          # S-box lookup tables [popSize, 2^inputs]
```

## Common Workflows

### Adding a New Experiment

1. Define genome and algorithm in `experiments.py`
2. Use `runs()` for single-criterion or `multicriterialRuns()` for multi-objective
3. Process results with `runsStats()` or `nondominance()`
4. Generate plots with `matplotlib` (pylab)
5. Format output as LaTeX tables

### Modifying Fitness Functions

1. Edit `prog/src/main/cpp/criterions.cpp` to add/modify criterion logic
2. Add enum to `CriterionFunction` in `prog/src/main/python/evolution.py`
3. Update `CRITERIONS_COUNT` constant
4. Rebuild C++ library: `cd prog && make clean && make`

### Adding New Genome Representation

1. Implement new genome class inheriting from `GAGenome` in C++
2. Add factory function in `prog/src/main/cpp/main.cpp`
3. Expose via `createGenome()` C interface
4. Add enum to `ChromozomeType` in `prog/src/main/python/evolution.py`

## Important Notes

### Python 3.14 Migration (January 2025)
- ✅ **Successfully migrated from Python 2.7 to Python 3.14**
- All syntax modernized: `print()` functions, `range()`, dict `.items()`
- Uses **uv** for package management (see `prog/pyproject.toml`)
- **Breaking change**: Python 2 pickle files are not compatible
- Set `SBOX_LIBRARY_PATH` environment variable for local development
- All unit tests pass successfully with pytest 8.3+
- Docker multi-stage build with test gates (Valgrind + Catch2 + pytest must all pass)

**Known Architectural Limitation**:
- Two C++ functions permanently disabled due to ctypes/Python C API incompatibility:
  - `simpleReportSearching()` - detailed evolution report (commented out)
  - `bestPopulationStringsSearching()` - string representation of S-boxes (returns empty list)
- **Root Cause**: Libraries loaded via `ctypes.CDLL` cannot safely call Python C API functions like `PyUnicode_FromStringAndSize()` to create Python objects. This would require converting the entire codebase to use Python extension modules instead of ctypes.
- **Workaround**: All core functionality works perfectly. Use `statistics().bestPopulationOutputs` (integer arrays) and `statistics().bestPopulationScores` instead of string representations.

### GAlib Integration
- The project relies on **GAlib 2.4.7** (included as tarball)
- GAlib must be compiled first in `../galib247/` directory
- Makefile expects specific directory structure

### Platform-Specific Optimizations
- Native makefile uses `-march=k8-sse3` (AMD K8 architecture from 2010)
- Adjust `-march` flag for your CPU architecture (Docker auto-patches to `-march=native`)
- OpenMP parallel execution assumes multi-core CPU

### Output Format
- Results saved as **pickle** files (Python 3 format)
- Plots generated as **EPS** (Encapsulated PostScript)
- LaTeX output uses Czech language strings

### Data Types
- S-boxes represented as lookup tables: `int[2^inputs]`
- Fitness values are `float32` (NumPy arrays)
- Criterion values stored as 8-element arrays (even if unused)

## Troubleshooting

### Compilation Errors (Native Build Only)
- **Missing GAlib**: Extract and compile `galib247bh100329.tar.bz2` first
- **Python headers**: Install `python3-dev` (3.14+) or adjust include path in makefile
- **OpenMP**: Ensure compiler supports `-fopenmp` flag
- **C++11 required**: Compiler must support `-std=gnu++11`
- **⚠️ Docker builds avoid all these issues automatically**

### Runtime Errors
- **Library not found**: Ensure `libsboxevolution.so` is compiled and run from `prog/` directory (or set `LD_LIBRARY_PATH`)
- **Segmentation fault**: Check genome parameter validity (e.g., input/output sizes)
- **Memory errors**: Large populations may exhaust RAM (reduce `popSize`)

### Test Errors
- **Docker builds failing at test stage**: Check build logs for Stage 5a (Valgrind), 5b (C++ tests), or 5c (Python tests)
- **C++ test compilation errors**: Ensure Catch2 header (`src/test/cpp/catch.hpp`) is present
- **ImportError: No module named evolution** (native): Must run tests from `prog/` directory
- **OSError: libsboxevolution.so: cannot open shared object file** (native): Compile C++ library first
- **Test failures after code changes**:
  - Docker: Rebuild with `docker-compose build --no-cache prog`
  - Native C++: `cd prog && make clean && make && make test`
  - Native Python: `cd prog && uv run pytest src/test/python/`
- **pytest not found** (native): Install with `uv sync` to install all dev dependencies

### Experiment Issues
- **No output directory**: Script creates `./results<timestamp>/` automatically
- **Missing plots**: Requires `matplotlib` (pylab) and display backend
- **LaTeX errors**: Tables use Czech characters, ensure UTF-8 encoding

### Docker Issues
- **Build failures**: Ensure Docker has sufficient memory (4GB+ recommended)
- **Permission errors**: Check that `artifacts/` directory is writable
- **Image size warnings**: Expected - TeX Live and build tools are large

## Research Context

This work explores the automated design of **cryptographic S-boxes** using evolutionary computation. S-boxes are critical components in symmetric encryption algorithms (e.g., DES, AES) that provide confusion in the cipher.

**Key Research Questions:**
1. Can evolutionary algorithms discover S-boxes competitive with hand-designed ones?
2. Which representation (binary, permutation, CGP) is most effective?
3. How do single-objective and multi-objective approaches compare?
4. What are optimal parameters for different algorithm types?

**Main Contributions:**
- Comparative study of 6+ evolutionary algorithms on S-box design
- Multi-objective optimization with 6 cryptographic criteria
- Novel CGP application to S-box evolution
- Extensive parameter sensitivity analysis (40 runs per configuration)

## References

- **Thesis PDF**: `DIPxhovor07final.pdf` (Czech language)
- **GAlib Documentation**: https://lancet.mit.edu/ga/
- **S-box Cryptanalysis**: See `text/crypto.utf8.tex` for background

## Version Information

- **Original Implementation**: January - May 2010
- **Author**: Bedrich Hovorka
- **Institution**: Brno University of Technology, Faculty of Information Technology
- **Degree**: Master's Thesis (2009/2010)
- **Repository Restructuring**: 2025
  - Source code reorganization (src/main/ and src/test/)
  - Docker containerization
  - Python unit test implementation
  - Python 2.7 to 3.14 migration, C++98 to C++11 upgrade
  - Documentation updates
- **Test Suite Extension**: March 2026
  - Catch2 v2 C++ unit tests (55 test cases, 1035 assertions)
  - pytest with hypothesis, pytest-benchmark, pytest-cov, pytest-xdist
  - Docker build gates: Valgrind + C++ tests + Python tests must all pass
  - On-demand C++ and Python performance benchmarks

## Code Quality Notes

**For Claude Code:**
- This is a legacy research codebase (2010) with modern enhancements (2025-2026)
- Fully migrated to Python 3.14 with modern syntax
- C++11 standard (`-std=gnu++11`) with GAlib 2.4.7 integration
- **All tests must pass before merging** — enforced by Docker build gates
- Comprehensive test suite: 55 C++ tests (Catch2 v2) + 26 Python tests + Valgrind
- 91% coverage on `evolution.py` (Python ctypes wrapper)
- Prefer Docker builds for consistency and reproducibility
- When modifying code:
  - Use Python 3.14 syntax and conventions
  - Use C++11 features (auto, range-for, unique_ptr, etc.)
  - Rebuild C++ library after any `.cpp` changes
  - **Run tests to verify changes**:
    - Docker: `docker-compose build prog` (all test gates run automatically)
    - Native C++: `make test` (Catch2 unit tests)
    - Native Python: `uv run pytest src/test/python/`
  - Add tests for new functionality (C++ in `src/test/cpp/test_*.cpp`, Python in `src/test/python/test_*.py`)
  - Update documentation if adding new features
