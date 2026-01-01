# S-box Evolver

**Design of S-Boxes Using Genetic Algorithms**

A research implementation of evolutionary algorithms for automated cryptographic S-box design and optimization.

---

## Overview

This project implements multiple evolutionary computation methods to search for optimal **Substitution boxes (S-boxes)** - fundamental components of symmetric encryption algorithms like DES and AES. The system evaluates S-box candidates against six cryptographic quality criteria using various genome representations and optimization strategies.

**Master's Thesis Project**
Brno University of Technology, Faculty of Information Technology
Author: Bedrich Hovorka
Year: 2009/2010

---

## Key Features

- **6 Evolutionary Algorithms**: GA, EDA (UMDA/BMDA), ParallelRandom, VEGA, SPEA, Random Search
- **4 Genome Representations**: Binary, Permutation, Cartesian Genetic Programming (CGP), Software Implementation
- **6 Cryptographic Criteria**: Linear/Differential Probability, SAC, Bentness, Balance Factor, Polynomial Degree
- **Multi-objective Optimization**: Pareto-optimal S-box discovery using VEGA and SPEA
- **Hybrid Architecture**: High-performance C++ core with Python experimentation interface
- **Comprehensive Experiments**: 6 pre-configured experimental scenarios with statistical analysis

---

## Quick Start

### Option 1: Docker Build (Recommended)

**No manual dependency installation required!**

```bash
# Build both C++ library and thesis PDF
docker-compose up --build

# Artifacts appear in ./artifacts/
ls -lh artifacts/prog/libsboxevolution.so
ls -lh artifacts/text/diplomka.pdf

# Or build individually
docker-compose build prog   # C++ library only
docker-compose build text   # Thesis PDF only
```

**What Docker does:**
- Automatically extracts and compiles GAlib 2.4.7
- Patches makefiles for modern compilers (g++-4.4 → g++, k8-sse3 → native, Python paths)
- Installs all Czech language tools (cstocs, vlna, texlive)
- Produces identical builds across platforms
- Eliminates all platform-specific limitations listed in this document

### Option 2: Native Build

**Prerequisites:**
- **C++ Compiler**: `g++ 4.4` or newer (C++98 support)
- **Python**: 2.5 or 2.6 (legacy codebase)
- **Libraries**: OpenMP, NumPy, Matplotlib (pylab)
- **Build Tools**: make
- **Optional**: LaTeX toolchain for thesis compilation (pdflatex, epstopdf, inkscape, dia)

**Installation:**

1. **Clone the repository**
   ```bash
   cd /path/to/sboxEvolver
   ```

2. **Extract and build GAlib** (one-time setup)
   ```bash
   tar -xjf galib247bh100329.tar.bz2
   cd galib247
   make
   cd ..
   ```

3. **Compile the evolution library**
   ```bash
   cd prog
   make
   ```

   This produces `libsboxevolution.so` - a shared library callable from Python.

   **Note:** `libsboxevolution.so` is a **build artifact** (not checked into git).

   **⚠️ You will likely need to adjust the makefile for modern systems:**
   - Line 18: Change `CXX = g++-4.4` to `CXX = g++`
   - Line 26: Change `-march=k8-sse3` to `-march=native`
   - Line 26: Update Python include path if using Python 2.7

   **These adjustments are automated by Docker - use Docker build to avoid manual patching.**

### Running an Experiment

```bash
cd prog
python src/main/python/experiments.py 1
```

**Available experiments:**
- `1` - Mutation/crossover parameter optimization for GA
- `2` - Genetic Algorithm vs. Estimation of Distribution Algorithm comparison
- `3` - ParallelRandom parameter sensitivity analysis
- `4` - VEGA and SPEA multi-objective parameter tuning
- `5` - Evolution of larger S-boxes (6x6, 8x8)
- `6` - Symbolic regression with CGP

**Output:** Creates `./results<YYYYMMDDHHMMSS>/` directory (in `prog/`) with:
- Statistical tables (LaTeX format)
- Box plot visualizations (EPS format)
- Serialized result data (pickle files)

---

## Thesis Compilation

The LaTeX thesis document is located in the `text/` directory.

### Docker Build (Recommended)

```bash
# Build thesis PDF in Docker (no LaTeX installation required)
docker-compose up --build text

# Output: ./artifacts/text/diplomka.pdf
```

### Native Build

```bash
cd text/
make
```

This will produce `diplomka.pdf` containing the complete thesis.

### Prerequisites

The following tools must be installed:

**Core LaTeX Tools:**
- `pdflatex` - LaTeX to PDF compiler
- `bibtex` - Bibliography processing

**Czech Language Support:**
- `cstocs` - Character encoding converter (UTF-8 → ISO-8859-2/IL2)
- `vlna` - Czech typography tool (non-breaking spaces)

**Image Processing:**
- `epstopdf` - EPS to PDF converter
- `inkscape` - SVG graphics processor
- `dia` - Diagram conversion tool

**Optional:**
- `gnuplot` - Plot generation (if regenerating figures)

### Installation Commands

**Fedora/RHEL:**
```bash
sudo dnf install texlive texlive-epstopdf inkscape dia
# Note: cstocs and vlna may require Czech-specific repositories
```

**Debian/Ubuntu:**
```bash
sudo apt install texlive-latex-base texlive-latex-extra texlive-bibtex-extra \
                 texlive-fonts-recommended texlive-lang-czechslovak \
                 epstopdf inkscape dia
```

### Build Targets

```bash
make           # Full build (default)
make rebuild   # Clean and rebuild
make clean     # Remove auxiliary files (.aux, .log, .toc, etc.)
make mrproper  # Remove all generated files including PDF
make images    # Regenerate images only
```

### Build Process Details

The Makefile orchestrates a multi-step build:

1. **Encoding Conversion**: Converts `.utf8.tex` files to ISO-8859-2 (Czech standard)
2. **Typography**: Applies Czech spacing rules using `vlna`
3. **Abbreviations**: Generates sorted abbreviation list from `zkratky.dat`
4. **Images**: Converts all EPS/SVG images to PDF format
5. **LaTeX Compilation**: Three-pass build process
   - Pass 1: Initial compilation
   - Pass 2: BibTeX bibliography processing
   - Pass 3-4: Cross-reference resolution

### Output

- **Primary output**: `diplomka.pdf` (full thesis document)
- **Language**: Czech
- **Source encoding**: UTF-8 (`.utf8.tex` files)
- **Output encoding**: ISO-8859-2 (Latin-2)

---

## Project Structure

```
sboxEvolver/
├── prog/                               # Source code
│   ├── Dockerfile                      # Multi-stage C++ build
│   ├── makefile                        # Build configuration
│   └── src/main/                       # Source files (reorganized)
│       ├── cpp/                        # C++ implementation
│       │   ├── main.cpp                # C++ core and algorithm implementations
│       │   ├── main.h                  # Main header
│       │   ├── common.h                # Common definitions
│       │   ├── cgp.cpp/h               # Cartesian Genetic Programming
│       │   ├── softwareSbox.cpp/h      # Optimized software S-box implementation
│       │   ├── eda.cpp/h               # Estimation of Distribution Algorithms
│       │   ├── criterions.cpp/h        # Cryptographic fitness functions
│       │   ├── multicriterial.cpp/h    # VEGA and SPEA implementations
│       │   ├── boxes.h                 # S-box lookup tables
│       │   └── combination.h           # Combinatorial utilities
│       └── python/                     # Python interface
│           ├── evolution.py            # Python wrapper (ctypes interface)
│           └── experiments.py          # 6 experimental scenarios
├── text/                               # LaTeX thesis document
│   ├── Dockerfile                      # LaTeX build with Czech tools
│   ├── diplomka.utf8.tex               # Main thesis file
│   ├── *.utf8.tex                      # Thesis chapters
│   └── img/                            # Figures and diagrams
├── artifacts/                          # Docker build outputs (gitignored)
│   ├── prog/
│   │   └── libsboxevolution.so        # Generated C++ shared library
│   └── text/
│       └── diplomka.pdf               # Generated thesis PDF
├── outputs/                            # Historical experiment results
├── docker-compose.yml                  # Build orchestration
├── galib247bh100329.tar.bz2           # GAlib 2.4.7 library
└── DIPxhovor07final.pdf               # Final thesis PDF (Czech)
```

---

## Architecture

### Hybrid Python/C++ Design

**C++ Core** (`libsboxevolution.so`):
- High-performance evolutionary operators (selection, crossover, mutation)
- Cryptographic quality evaluators
- Parallel execution via OpenMP
- Built on GAlib genetic algorithm framework

**Python Interface** (`evolution.py`):
- Ctypes bindings to C++ library
- Convenient API for experiments
- Statistical analysis and visualization
- Experiment orchestration

### Key Components

#### 1. Genome Representations

| Type | Description | Use Case |
|------|-------------|----------|
| **BINARY** | Binary string encoding | Traditional GA benchmarking |
| **PERMUTATION** | Integer permutation | Bijective S-boxes (n→n mappings) |
| **CGP** | Cartesian Genetic Programming | Evolved boolean circuits |
| **SOFTWARE_IMPL** | Optimized lookup table | Fast software implementations |

#### 2. Cryptographic Fitness Criteria

| Criterion | Description | Optimization Goal |
|-----------|-------------|-------------------|
| **LP_MAX** | Maximum Linear Probability | Minimize (resistance to linear cryptanalysis) |
| **DP_MAX** | Maximum Differential Probability | Minimize (resistance to differential cryptanalysis) |
| **SAC** | Strict Avalanche Criterion | Maximize (output avalanche effect) |
| **BENT_AND_MOSAC** | Bentness + Max-order SAC | Maximize (nonlinearity) |
| **BF** | Balance and Completeness Factor | Maximize (bit distribution) |
| **POLYNOMIAL_DEGREE** | Algebraic degree | Maximize (algebraic complexity) |

#### 3. Evolutionary Algorithms

- **Ga**: Canonical Genetic Algorithm with elitism
- **Random**: Baseline random search
- **ParallelRandom**: Mutate-best-in-parallel strategy
- **Eda**: Estimation of Distribution (UMDA/BMDA)
- **Vega**: Vector Evaluated GA (multi-objective)
- **Spea**: Strength Pareto EA (multi-objective)

---

## Usage Examples

### Example 1: Single-Criterion Optimization

```python
from evolution import *

# Define 4x4 permutation S-box optimized for differential resistance
genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.DP_MAX, True, 4, 4)

# Genetic algorithm with tuned parameters
search = Searching("Ga", genome,
                   popSize=1000,
                   nGen=50,
                   pMut=0.1,
                   pCross=0.9,
                   elitism=False)

# Execute evolution
search.simpleEvolve(TerminationCondition.GENERATION)

# Retrieve best solutions
stats = search.statistics()
print("Best fitness:", stats.maxEver)
print("Best S-box:", search.bestPopulationStrings()[0])

search.close()
```

### Example 2: Multi-Objective Optimization

```python
from evolution import *

# Define multiple cryptographic objectives
criterions = [
    CriterionFunction.LP_MAX,
    CriterionFunction.DP_MAX,
    CriterionFunction.SAC
]

# CGP representation for 4x4 S-box
genome = Genome(ChromozomeType.CGP, CriterionFunction.NONE_FITNESS,
                4, 4, 4, 3, 6)

# SPEA multi-objective optimization
search = Searching("Spea", genome, 1000, 50, 0.05, 0.7, criterions)
search.simpleEvolve()

# Extract Pareto front
stats = search.statistics()
pareto_scores = stats.bestPopulationCriterionsValues
print("Pareto front size:", len(pareto_scores))

search.close()
```

### Example 3: Parameter Sensitivity Study

```python
from evolution import *

# Test multiple mutation rates
results = {}
for pMut in [0.01, 0.05, 0.1, 0.2, 0.5]:
    genome = Genome(ChromozomeType.BINARY, CriterionFunction.SAC, 4, 4)

    # Run 40 independent trials
    searches = [Searching("ParallelRandom", genome, 1000, 50, pMut)
                for _ in range(40)]

    Searching.parallelSearching(searches, TerminationCondition.GENERATION)

    scores = [s.statistics().maxEver for s in searches]
    results[pMut] = scores

    for s in searches:
        s.close()

# Analyze results
for pMut, scores in results.items():
    print(f"pMut={pMut}: mean={numpy.mean(scores):.3f} std={numpy.std(scores):.3f}")
```

---

## Experimental Results

The thesis includes comprehensive experiments comparing:

1. **Parameter Optimization**: Mutation/crossover probability tuning for GA (Experiment 1)
2. **Algorithm Comparison**: GA vs. EDA on binary representation (Experiment 2)
3. **Representation Comparison**: Parallel Random on CGP vs. Software Implementation (Experiment 3)
4. **Multi-objective Tuning**: VEGA and SPEA parameter sensitivity (Experiment 4)
5. **Scalability**: Evolution of 6x6 and 8x8 S-boxes (Experiment 5)
6. **Symbolic Regression**: CGP-based algebraic S-box discovery (Experiment 6)

**Key Findings** (from thesis):
- Permutation representation outperforms binary encoding for bijective S-boxes
- EDA (especially BMDA) competitive with tuned GA
- Multi-objective approaches (VEGA/SPEA) discover diverse Pareto-optimal solutions
- CGP enables evolution of compact boolean circuit implementations

---

## Citation

If you use this work in research, please cite:

```
Hovorka, B. (2010). Design of S-Boxes Using Genetic Algorithms.
Master's Thesis, Brno University of Technology, Faculty of Information Technology.
```

---

## Docker Details

### Architecture

The project uses a **build-only container pattern**:

1. **prog/Dockerfile** (multi-stage):
   - Stage 1: Build GAlib 2.4.7 from tarball
   - Stage 2: Compile libsboxevolution.so with auto-patched makefile
   - Stage 3: Runtime with Python 2.7 + NumPy + Matplotlib
   - Copies `.so` to mounted volume and exits

2. **text/Dockerfile** (single-stage):
   - Installs full TeX Live + Czech tools (cstocs, vlna)
   - Compiles thesis with 4-pass LaTeX build
   - Copies `diplomka.pdf` to mounted volume and exits

### Automatic Makefile Patching

Docker automatically applies these fixes for modern systems:
```bash
# Compiler version (g++-4.4 → g++)
sed -i 's|CXX = g++-4.4|CXX = g++|g' makefile

# CPU architecture (AMD K8 2010 → modern CPU)
sed -i 's/-march=k8-sse3/-march=native/g' makefile

# Python headers (2.5 → 2.7)
sed -i 's|-I/usr/local/include/python2.5|-I/usr/include/python2.7|g' makefile
```

This eliminates the need for manual makefile editing and resolves platform-specific limitations.

### Build Validation

```bash
# Check library symbols
nm -D artifacts/prog/libsboxevolution.so | grep "newGaSearching"

# Check PDF validity
pdfinfo artifacts/text/diplomka.pdf
```

### Security Note

Docker images use legacy Python 2.7 and old Debian versions with known CVEs. **Only use for local builds, not production deployment.**

## Known Limitations

- **Python 2 Only**: Code written for Python 2.5/2.6 (requires porting for Python 3+)
- **Legacy Dependencies**: GAlib 2.4.7 (2001), C++98 standard
- **Platform-Specific** (Native builds only): Makefile hardcodes AMD K8 architecture flags and Python 2.5 paths
  - **✓ Resolved by Docker**: Automatic patching for modern compilers and architectures
- **Language**: Thesis and some output strings in Czech
- **Docker images**: Contain unpatched security vulnerabilities (local use only)

---

## Future Work

Potential extensions and modernization:
- Port to Python 3.x
- Replace GAlib with modern C++17/20 framework
- Add GPU-accelerated fitness evaluation
- Implement additional representations (e.g., neural architecture search)
- Extend to AES-scale S-boxes (8→8 bits)
- Interactive visualization of Pareto fronts

---

## License

This is academic research software from 2010. Check with the original author (Bedrich Hovorka) or Brno University of Technology regarding usage rights.

---

## Acknowledgments

- **Supervisor**: (see thesis for details)
- **GAlib**: Matthew Wall, MIT Lincoln Laboratory
- **Institution**: Brno University of Technology, Faculty of Information Technology

---

## Contact

For questions about this historical codebase, refer to the thesis document (`DIPxhovor07final.pdf`) or contact Brno University of Technology's Faculty of Information Technology.

---

**Note**: This is a legacy research project from 2010. While historically interesting, consider modern alternatives (e.g., CMA-ES, NSGA-III, neural architecture search) for contemporary S-box design research.
