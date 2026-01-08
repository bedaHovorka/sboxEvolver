# S-box Evolver

**Design of S-Boxes Using Genetic Algorithms**

A research implementation of evolutionary algorithms for automated cryptographic S-box design and optimization.

![Platform](https://img.shields.io/badge/platform-Linux-blue)
![Language](https://img.shields.io/badge/python-3.13-yellow)
![C++](https://img.shields.io/badge/C++-98-green)
![License](https://img.shields.io/badge/license-Academic-lightgrey)
![Package Manager](https://img.shields.io/badge/uv-package%20manager-purple)

---

## Overview

This project implements multiple evolutionary computation methods to search for optimal **Substitution boxes (S-boxes)** - fundamental components of symmetric encryption algorithms like DES and AES. The system evaluates S-box candidates against six cryptographic quality criteria using various genome representations and optimization strategies.

**Master's Thesis Project**
Brno University of Technology, Faculty of Information Technology
Author: Bedrich Hovorka
Year: 2009/2010

**Recent Updates (2025):**
- ✅ **Migrated from Python 2.7 to Python 3.13** (January 2025)
- Modern Python syntax and uv package manager
- Separated C++ and Python source files
- Added Docker support for reproducible builds
- Implemented Python unit tests with pytest 8.3+
- Updated documentation

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
- Patches makefiles for modern compilers (g++-4.4 → g++, k8-sse3 → native)
- Configures Python 3.13 environment with pytest 8.3+
- Installs all Czech language tools (cstocs, vlna, texlive)
- Produces identical builds across platforms
- Eliminates all platform-specific limitations listed in this document

### Option 2: Local Development (Python 3.13 + uv)

**For running experiments on your host machine:**

```bash
# 1. Build C++ library using Docker (one-time setup)
docker-compose build prog

# 2. Install Python dependencies with uv
cd prog/
uv sync

# 3. Set library path to Docker-compiled artifact
export SBOX_LIBRARY_PATH=/path/to/sboxEvolver/artifacts/prog

# 4. Run experiments
uv run python src/main/python/experiments.py 1

# 5. Run tests
uv run pytest src/test/python/
```

**Prerequisites:**
- Docker (for C++ library compilation)
- Python 3.13+
- uv package manager (`pip install uv`)

**Benefits:**
- Use your native Python environment
- Fast iteration (no Docker rebuild needed)
- Access to all Python debugging tools
- Consistent C++ library from Docker

### Option 3: Native Build (Advanced)

**⚠️ Not recommended:** Use Docker (Option 1) or Local Development (Option 2) instead.

**Prerequisites:**
- **C++ Compiler**: `g++` with C++98 support
- **Python**: 3.13+ with development headers
- **Python Libraries**: Install via uv (`uv sync` in `prog/`)
- **System Libraries**: OpenMP (`libgomp`)
- **Build Tools**: make
- **GAlib 2.4.7**: Must be extracted and compiled manually
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
   - Python include paths: Update to your Python 3.13 installation

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

### Running Unit Tests

The project includes Python unit tests for core functionality.

**Docker (Recommended):**

Tests are automatically run during the Docker build process using pytest:

```bash
# Tests run automatically as part of the build
docker-compose build prog

# The build will fail if tests don't pass
# Tests are executed in Stage 5 of the multi-stage Dockerfile
```

The Docker build uses pytest 8.3+ with Python 3.13:
- Tests run in an isolated environment with all dependencies
- Compiled `libsboxevolution.so` is automatically available
- Uses non-interactive matplotlib backend (Agg)

**Native (Manual):**

If running tests without Docker (requires compiled library):

```bash
cd prog

# Ensure library path is set
export SBOX_LIBRARY_PATH=/path/to/sboxEvolver/artifacts/prog

# Run all tests with pytest via uv
uv run pytest src/test/python/

# Or with pytest directly (if installed)
pytest src/test/python/

# Run specific test file
uv run pytest src/test/python/test_routines.py
uv run pytest src/test/python/test_experiments.py

# Run with verbose output
uv run pytest -v src/test/python/
```

**Test Coverage:**
- S-box lookup table operations
- Cryptographic criterion calculations
- Genome creation and manipulation
- Basic evolution workflows

**Prerequisites for native testing:**
- Compiled `libsboxevolution.so` must be present
- Python 3.13 with numpy and matplotlib
- pytest 8.3+ and assertpy 1.1+ (installed via `uv sync`)

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
   - Stage 3: Build Python 3.13 dependencies (numpy with gcc/g++)
   - Stage 4: Clean Python 3.13 runtime (libgomp1 only, no build tools)
   - Stage 5: Run pytest 8.3+ tests
   - Stage 6: Production runtime, copies `.so` to mounted volume and exits

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

# Python 3.13 headers
# Configured automatically based on Docker Python 3.13 installation
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

Docker images are based on older Debian versions. **Only use for local builds and research, not production deployment.**

## Testing

### Unit Tests

Python unit tests validate core functionality using pytest (in Docker) or unittest (native).

**Docker Testing (Recommended):**

Tests run automatically during Docker build:

```bash
# Tests run as part of Stage 4 in multi-stage build
docker-compose build prog

# Build fails if tests don't pass
# Uses: python -m pytest prog/src/test/python
```

**Native Testing (Manual):**

```bash
cd prog

# With pytest (if installed)
pytest src/test/python/

# Or with unittest
python -m unittest discover -s src/test/python/ -p "test_*.py" -v

# Run specific test suite
python -m unittest src.test.python.test_routines
python -m unittest src.test.python.test_experiments
```

**Test Organization:**
```
prog/src/test/python/
├── test_routines.py      # Core algorithm tests
└── test_experiments.py   # Experiment framework tests
```

**Docker Test Environment:**
- pytest 8.3+ (Python 3.13 compatible)
- assertpy 1.1+ (assertion library)
- Isolated build with all dependencies
- Non-interactive matplotlib backend

**Requirements for Native Testing:**
- Compiled `libsboxevolution.so` must be available
- Run from `prog/` directory to ensure library path resolution
- Python 3.13 with numpy and matplotlib
- pytest 8.3+ and assertpy 1.1+ (installed via `uv sync`)

### Integration Testing

Run experiments with reduced parameters for quick validation:

```python
# Modify experiments.py temporarily
genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.LP_MAX, True, 4, 4)
search = Searching("Ga", genome, popSize=50, nGen=5, pMut=0.1, pCross=0.9)
```

## Known Limitations

- **✓ Migrated to Python 3.13** (January 2025): Successfully ported from Python 2.x
  - **Architectural Constraint**: Two C++ functions (`simpleReportSearching()`, `bestPopulationStringsSearching()`) disabled due to ctypes/Python C API incompatibility. Use `statistics().bestPopulationOutputs` instead.
- **Legacy Dependencies**: GAlib 2.4.7 (2001), C++98 standard
- **Platform-Specific** (Native builds only): Makefile hardcodes AMD K8 architecture flags
  - **✓ Resolved by Docker**: Automatic patching for modern compilers and architectures
- **Language**: Thesis and some output strings in Czech
- **Docker images**: Based on older Debian versions - local use only, not for production
- **Unit Tests**: Using pytest 8.3+ (Python 3.13 compatible)

---

## Future Work

Potential extensions and modernization:
- **✓ Port to Python 3.x** (Completed January 2025)
- Convert ctypes library to proper Python extension module (would enable all C API functions)
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

## Recent Changes (2025)

This repository has been updated from the original 2010 version:

- **Source Restructuring**: Organized into `src/main/cpp/` and `src/main/python/`
- **Docker Support**: Added multi-stage builds for reproducible compilation
- **Unit Testing**: Implemented Python unit tests for core components
- **Documentation**: Enhanced README and added CLAUDE.md for AI assistant guidance
- **GitHub Copilot Instructions**: Added `.github/copilot-instructions.md` for AI-assisted development
- **Build Artifacts**: Separated generated files into `artifacts/` directory

Original thesis and results remain unchanged. See git history for restructuring details.

## Contact

**Original Author**: Bedrich Hovorka
**Institution**: Brno University of Technology, Faculty of Information Technology
**Thesis Document**: `DIPxhovor07final.pdf` (Czech language)

For questions about this historical codebase, refer to the thesis document or contact the Faculty of Information Technology.

---

**Note**: This is a legacy research project from 2010, restructured in 2025. While historically interesting, consider modern alternatives (e.g., CMA-ES, NSGA-III, neural architecture search) for contemporary S-box design research.
