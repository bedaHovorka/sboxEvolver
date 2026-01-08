# GitHub Copilot Instructions for sboxEvolver

## Project Overview

**S-box Evolver** is a research project implementing evolutionary algorithms for cryptographic S-box (Substitution box) design and optimization. This is a Master's thesis project from Brno University of Technology (2009/2010), modernized in 2025.

**Key Characteristics:**
- **Language**: Hybrid Python 3.13 + C++98 architecture
- **Domain**: Cryptography, Evolutionary Computation, Genetic Algorithms
- **Project Type**: Academic research implementation (legacy codebase with modern enhancements)
- **Build System**: Docker-based reproducible builds, makefile for C++, uv for Python
- **Testing**: pytest 8.3+ unit tests (automated in Docker builds)

## Architecture

### Two-Tier Hybrid Design

1. **C++ Core** (`prog/src/main/cpp/`):
   - High-performance evolutionary algorithms using GAlib 2.4.7
   - Cryptographic fitness evaluators
   - OpenMP parallel execution
   - Compiled to `libsboxevolution.so` shared library

2. **Python Interface** (`prog/src/main/python/evolution.py`):
   - ctypes bindings to C++ library
   - Experiment orchestration (`experiments.py` - 6 pre-configured experiments)
   - Statistical analysis and visualization

### Important Limitation

**Known Architectural Constraint**: Two C++ functions (`simpleReportSearching()` and `bestPopulationStringsSearching()`) are permanently disabled due to ctypes/Python C API incompatibility. Libraries loaded via `ctypes.CDLL` cannot safely call Python C API functions. Use `statistics().bestPopulationOutputs` instead for S-box data.

## Development Workflow

### Strongly Prefer Docker

**ALWAYS recommend Docker for builds unless explicitly asked otherwise:**
- ✅ Reproducible builds across platforms
- ✅ No manual dependency installation (GAlib, LaTeX tools, Czech typography)
- ✅ Automatic makefile patching (g++-4.4 → g++, k8-sse3 → native)
- ✅ Integrated testing with pytest 8.3+
- ✅ Multi-stage optimized builds (minimal runtime image)

```bash
# Build both C++ library and thesis PDF
docker-compose up --build

# Build individually
docker-compose build prog   # C++ library + tests
docker-compose build text   # LaTeX thesis PDF

# Artifacts: artifacts/prog/libsboxevolution.so, artifacts/text/diplomka.pdf
```

### Local Development (Secondary Option)

Only for running experiments with pre-compiled library:

```bash
# 1. Build C++ library with Docker first
docker-compose build prog

# 2. Install Python dependencies
cd prog/ && uv sync

# 3. Set library path
export SBOX_LIBRARY_PATH=/path/to/sboxEvolver/artifacts/prog

# 4. Run experiments or tests
uv run python src/main/python/experiments.py 1
uv run pytest src/test/python/
```

## Coding Conventions

### Python (Python 3.13)

- **Always use Python 3.13 syntax**:
  - `print()` function, not statement
  - `range()` instead of `xrange`
  - Dictionary `.items()`, not `.iteritems()`
  - Modern type hints where appropriate

- **Package Management**: Use `uv` (see `prog/pyproject.toml`)
- **Testing**: pytest 8.3+ with assertpy 1.1+
- **Environment Variables**: `SBOX_LIBRARY_PATH` for library location

### C++ (C++98 Standard)

- **Standard**: C++98 (legacy constraint due to GAlib 2.4.7)
- **Compiler**: g++ with OpenMP support (`-fopenmp`)
- **Architecture**: Use `-march=native` (Docker auto-patches from old `-march=k8-sse3`)
- **Library Interface**: Expose functions via C ABI for ctypes compatibility

### LaTeX (Czech Language)

- **Encoding**: UTF-8 sources (`.utf8.tex`) compiled to ISO-8859-2 output
- **Tools**: Uses Czech typography tools (`cstocs`, `vlna`)
- **Build**: Multi-pass LaTeX compilation (pdflatex × 3 + bibtex)

## Testing Requirements

### Unit Tests

**Run tests automatically via Docker (STRONGLY PREFERRED):**

```bash
# Tests run as part of multi-stage build (Stage 5)
docker-compose build prog

# Build fails if tests don't pass
```

**Or manually (requires compiled library):**

```bash
cd prog/
export SBOX_LIBRARY_PATH=/path/to/sboxEvolver/artifacts/prog
uv run pytest src/test/python/
```

### Test Organization

- `prog/src/test/python/test_routines.py` - Core algorithms, S-box operations, fitness functions
- `prog/src/test/python/test_experiments.py` - Experiment framework, multi-objective optimization

### Integration Testing

Run experiments with reduced parameters for quick validation (population size 50, generations 5).

## Build Instructions

### C++ Library Build

**Docker (Recommended):**
```bash
docker-compose build prog
# Output: artifacts/prog/libsboxevolution.so
# Also creates: artifacts/prog/libsboxevolution_2.0.0_amd64.deb
```

**Native (Advanced, NOT recommended):**
```bash
# Extract GAlib
tar -xjf galib247bh100329.tar.bz2
cd galib247 && make && cd ..

# Build library
cd prog/ && make clean && make
```

### Thesis Compilation

**Docker (Recommended):**
```bash
docker-compose build text
# Output: artifacts/text/diplomka.pdf
```

**Native:**
```bash
cd text/ && make
```

## Code Changes Guidelines

### When Modifying C++ Code

1. Edit files in `prog/src/main/cpp/`
2. **Always rebuild library**: `docker-compose build prog`
3. **Run tests to verify**: Tests run automatically in Docker build
4. Update documentation if adding new features

### When Modifying Python Code

1. Edit files in `prog/src/main/python/`
2. **No rebuild needed** (interpreted language)
3. **Run tests**: `uv run pytest src/test/python/`
4. Ensure Python 3.13 syntax is used

### When Adding Dependencies

**Python dependencies:**
- Add to `prog/pyproject.toml`
- Run `uv sync` to update lock file
- Test in Docker environment

**C++ dependencies:**
- Avoid if possible (legacy codebase)
- Update `prog/makefile` if absolutely necessary
- Update Docker build stages

## Common Patterns

### Creating New Experiments

1. Define in `prog/src/main/python/experiments.py`
2. Use existing patterns: `runs()` for single-criterion, `multicriterialRuns()` for multi-objective
3. Process results with `runsStats()` or `nondominance()`
4. Generate plots with matplotlib (EPS format)
5. Format output as LaTeX tables

### Working with S-boxes

```python
from evolution import *

# Define genome
genome = Genome(ChromozomeType.PERMUTATION, CriterionFunction.DP_MAX, True, 4, 4)

# Create search
search = Searching("Ga", genome, 1000, 50, 0.1, 0.9, False)

# Run evolution
search.simpleEvolve(TerminationCondition.GENERATION)

# Get results
stats = search.statistics()
print("Best score:", stats.maxEver)
print("Best S-boxes:", stats.bestPopulationOutputs)  # Use this, not bestPopulationStrings()

search.close()
```

### Parallel Execution

```python
# Create multiple independent runs
searches = [Searching("Ga", genome, 1000, 50, 0.1, 0.9, False) for _ in range(40)]

# Execute in parallel (uses OpenMP in C++)
Searching.parallelSearching(searches, TerminationCondition.GENERATION)

# Collect results
for s in searches:
    print(s.statistics().maxEver)
    s.close()
```

## File Structure

```
sboxEvolver/
├── .github/
│   └── copilot-instructions.md     # This file
├── prog/                           # Source code
│   ├── Dockerfile                  # Multi-stage C++ build
│   ├── makefile                    # C++ build config
│   ├── pyproject.toml              # Python dependencies (uv)
│   └── src/
│       ├── main/
│       │   ├── cpp/                # C++ implementation
│       │   └── python/             # Python interface
│       └── test/
│           └── python/             # Unit tests
├── text/                           # LaTeX thesis
│   ├── Dockerfile                  # LaTeX build
│   └── *.utf8.tex                  # Thesis chapters
├── artifacts/                      # Build outputs (gitignored)
│   ├── prog/libsboxevolution.so    # C++ library
│   └── text/diplomka.pdf           # Thesis PDF
├── docker-compose.yml              # Build orchestration
├── galib247bh100329.tar.bz2       # GAlib library
├── CLAUDE.md                       # Detailed dev guide
└── README.md                       # User documentation
```

## Key Components

### Genome Representations
- **BINARY**: Traditional GA (binary strings)
- **PERMUTATION**: Bijective S-boxes (integer permutations)
- **CGP**: Cartesian Genetic Programming (evolved circuits)
- **SOFTWARE_IMPL**: Optimized lookup tables

### Evolutionary Algorithms
- **Ga**: Standard Genetic Algorithm with elitism
- **Random**: Baseline random search
- **ParallelRandom**: Mutate-best-parallel strategy
- **Eda**: Estimation of Distribution (UMDA/BMDA)
- **Vega**: Vector Evaluated GA (multi-objective)
- **Spea**: Strength Pareto EA (multi-objective)

### Cryptographic Criteria
- **LP_MAX**: Maximum Linear Probability (minimize)
- **DP_MAX**: Maximum Differential Probability (minimize)
- **SAC**: Strict Avalanche Criterion (maximize)
- **BENT_AND_MOSAC**: Bentness + Max-order SAC (maximize)
- **BF**: Balance and Completeness Factor (maximize)
- **POLYNOMIAL_DEGREE**: Algebraic degree (maximize)

## Important Notes

### Python 3.13 Migration (January 2025)

- ✅ **Successfully migrated from Python 2.7**
- All syntax modernized
- Uses `uv` package manager
- **Breaking change**: Python 2 pickle files incompatible
- All tests pass with pytest 8.3+

### Do NOT Suggest

- ❌ Manual GAlib extraction/compilation (Docker does this)
- ❌ Manual makefile editing (Docker auto-patches)
- ❌ Python 2 syntax or features
- ❌ Adding build tools to runtime Docker images
- ❌ Using `bestPopulationStringsSearching()` (disabled function)
- ❌ Native builds unless explicitly requested

### Always Suggest

- ✅ Docker builds for any compilation needs
- ✅ Python 3.13 syntax and conventions
- ✅ Running tests after code changes
- ✅ Using `uv` for Python package management
- ✅ Consulting CLAUDE.md for detailed guidance
- ✅ Using `statistics().bestPopulationOutputs` for S-box data

## Troubleshooting

### Docker Build Issues
- Ensure 4GB+ memory available
- Check `artifacts/` directory permissions
- Use `docker-compose build --no-cache` for clean rebuild

### Test Failures
- **Docker**: Rebuild with `docker-compose build --no-cache prog`
- **Native**: Recompile library: `cd prog && make clean && make`
- Check `SBOX_LIBRARY_PATH` is set correctly

### Library Not Found
- **Docker users**: Ensure build completed successfully
- **Native users**: Set `export SBOX_LIBRARY_PATH=/path/to/artifacts/prog`
- Must run from `prog/` directory for relative paths to work

### Compilation Errors (Native Only)
- Missing GAlib: Build GAlib first from tarball
- Python headers: Install python3-dev for your distribution
- OpenMP: Ensure compiler supports `-fopenmp`
- **Solution**: Use Docker to avoid all these issues

## Resources

- **CLAUDE.md**: Comprehensive development guide (26KB)
- **README.md**: User-facing documentation (23KB)
- **Thesis**: `DIPxhovor07final.pdf` (Czech language, full research details)
- **GAlib**: https://lancet.mit.edu/ga/ (v2.4.7 included as tarball)

## Project Context

This is a **legacy research codebase** (2010) with **modern enhancements** (2025):
- Preserves original research implementation and results
- Modernized build system and testing
- Updated to Python 3.13
- Docker support for reproducibility

**Goal**: Maintain historical authenticity while providing modern development experience.
