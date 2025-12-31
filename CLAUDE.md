# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

**S-box Evolver** - A research project implementing evolutionary algorithms for the design and optimization of cryptographic S-boxes (Substitution boxes).

This is a Master's thesis project from Brno University of Technology, Faculty of Information Technology (2009/2010), authored by Bedrich Hovorka. The project explores using genetic algorithms and other evolutionary computation methods to search for optimal S-boxes based on multiple cryptographic criteria.

## Project Structure

```
sboxEvolver/
├── prog/                      # Source code (Python + C++)
│   ├── evolution.py          # Python wrapper for evolutionary algorithms
│   ├── experiments.py        # Experimental framework with 6 experiments
│   ├── main.cpp              # C++ core implementation (GAlib integration)
│   ├── cgp.cpp/h             # Cartesian Genetic Programming representation
│   ├── softwareSbox.cpp/h    # Software implementation of S-boxes
│   ├── eda.cpp/h             # Estimation of Distribution Algorithms
│   ├── criterions.cpp/h      # Cryptographic fitness functions
│   ├── multicriterial.cpp/h  # VEGA and SPEA algorithms
│   ├── boxes.h               # S-box lookup tables
│   ├── combination.h         # Combinatorial utilities
│   ├── makefile              # Build configuration
│   └── libsboxevolution.so   # Compiled shared library
├── text/                      # LaTeX thesis document
│   ├── *.utf8.tex            # Thesis chapters
│   ├── diplomka.bib          # Bibliography
│   ├── img/                  # Figures and diagrams
│   └── Makefile              # LaTeX build
├── outputs/                   # Experiment results and logs
├── galib247bh100329.tar.bz2  # GAlib library (genetic algorithm framework)
├── DIPxhovor07final.pdf      # Final thesis PDF
└── README.txt                # Original Czech readme

```

## Architecture

### Hybrid Python/C++ Design

The project uses a **two-tier architecture**:

1. **C++ Core** (`prog/*.cpp/h`): High-performance evolutionary algorithms using GAlib
2. **Python Interface** (`prog/evolution.py`): Wraps C++ via ctypes for easy experimentation

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

### Building C++ Library

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

# This produces libsboxevolution.so
```

**Dependencies:**
- `g++-4.4` or newer (C++98 standard)
- OpenMP support (`-fopenmp`)
- Python 2.5/2.6 development headers
- GAlib 2.4.7 (included as tarball)

### Running Experiments

The `experiments.py` file contains 6 numbered experiments from the thesis:

```bash
cd prog/

# Run specific experiment (1-6)
python experiments.py <experiment_number>

# Examples:
python experiments.py 1  # Mutation/crossover parameter tuning
python experiments.py 2  # GA vs. EDA comparison
python experiments.py 3  # ParallelRandom parameter search
python experiments.py 4  # VEGA/SPEA parameter tuning
python experiments.py 5  # Larger S-boxes (6x6, 8x8)
python experiments.py 6  # Symbolic regression
```

**Output:**
- Creates timestamped directory: `./results<YYYYMMDDHHMMSS>/`
- Generates EPS plots and LaTeX tables
- Saves serialized results (`.picle` files)

### Thesis Compilation

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

1. Edit `prog/criterions.cpp` to add/modify criterion logic
2. Add enum to `CriterionFunction` in `evolution.py`
3. Update `CRITERIONS_COUNT` constant
4. Rebuild C++ library: `make clean && make`

### Adding New Genome Representation

1. Implement new genome class inheriting from `GAGenome` in C++
2. Add factory function in `main.cpp`
3. Expose via `createGenome()` C interface
4. Add enum to `ChromozomeType` in `evolution.py`

## Important Notes

### Python 2 Legacy Code
- This project was written for **Python 2.5/2.6**
- Uses `xrange`, `print` statements, old-style string formatting
- To run on modern Python 3, significant porting would be required:
  - `print` → `print()`
  - `xrange` → `range`
  - Dictionary `.iteritems()` → `.items()`
  - ctypes string handling changes

### GAlib Integration
- The project relies on **GAlib 2.4.7** (included as tarball)
- GAlib must be compiled first in `../galib247/` directory
- Makefile expects specific directory structure

### Platform-Specific Optimizations
- Makefile uses `-march=k8-sse3` (AMD K8 architecture)
- Adjust `-march` flag for your CPU architecture
- OpenMP parallel execution assumes multi-core CPU

### Output Format
- Results saved as **pickle** files (Python 2 format)
- Plots generated as **EPS** (Encapsulated PostScript)
- LaTeX output uses Czech language strings

### Data Types
- S-boxes represented as lookup tables: `int[2^inputs]`
- Fitness values are `float32` (NumPy arrays)
- Criterion values stored as 8-element arrays (even if unused)

## Troubleshooting

### Compilation Errors
- **Missing GAlib**: Extract and compile `galib247bh100329.tar.bz2` first
- **Python headers**: Install `python2-dev` or adjust include path in makefile
- **OpenMP**: Ensure compiler supports `-fopenmp` flag

### Runtime Errors
- **Library not found**: Run from `prog/` directory or set `LD_LIBRARY_PATH`
- **Segmentation fault**: Check genome parameter validity (e.g., input/output sizes)
- **Memory errors**: Large populations may exhaust RAM (reduce `popSize`)

### Experiment Issues
- **No output directory**: Script creates `./results<timestamp>/` automatically
- **Missing plots**: Requires `matplotlib` (pylab) and display backend
- **LaTeX errors**: Tables use Czech characters, ensure UTF-8 encoding

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

- **Created**: January 2010
- **Last Modified**: May 23, 2010
- **Author**: Bedrich Hovorka
- **Institution**: Brno University of Technology, Faculty of Information Technology
- **Degree**: Master's Thesis (2009/2010)
