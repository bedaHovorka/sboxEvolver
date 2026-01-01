#
#      Brno University of Technology
#      Faculty of Information Technology
#
#      MSc Thesis       2009/2010
#
#      Design of S-Boxes Using Genetic Algorithms
#
#      Dockerization: 2025
#
#      Multi-stage build for libsboxevolution.so
#

# ============================================
# Stage 1: Build GAlib 2.4.7
# ============================================
FROM debian:buster-slim AS galib-builder

# Debian Buster is archived - update sources to use archive.debian.org
RUN sed -i 's|http://deb.debian.org|http://archive.debian.org|g' /etc/apt/sources.list && \
    sed -i 's|http://security.debian.org|http://archive.debian.org|g' /etc/apt/sources.list && \
    sed -i '/buster-updates/d' /etc/apt/sources.list

# Install build dependencies for GAlib
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    tar \
    bzip2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy and extract GAlib tarball from project root
COPY galib247bh100329.tar.bz2 .
RUN tar -xjf galib247bh100329.tar.bz2

# Build GAlib static library
WORKDIR /build/galib247

# Patch GAlib for modern C++ compilers (add this-> prefix for template member functions)
RUN for file in ga/GA1DArrayGenome.C ga/GA2DArrayGenome.C ga/GA3DArrayGenome.C; do \
        sed -i 's/^  initializer(/  this->initializer(/g' "$file"; \
        sed -i 's/^  mutator(/  this->mutator(/g' "$file"; \
        sed -i 's/^  comparator(/  this->comparator(/g' "$file"; \
        sed -i 's/^  crossover(/  this->crossover(/g' "$file"; \
    done

RUN make

# ============================================
# Stage 2: Build libsboxevolution.so
# ============================================
FROM python:2.7-slim AS sbox-builder

# Python 2.7-slim is based on Debian Buster (archived) - update sources
RUN sed -i 's|http://deb.debian.org|http://archive.debian.org|g' /etc/apt/sources.list && \
    sed -i 's|http://security.debian.org|http://archive.debian.org|g' /etc/apt/sources.list && \
    sed -i '/buster-updates/d' /etc/apt/sources.list

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libgomp1 \
    python2.7-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build/sboxEvolver

# Copy compiled GAlib from previous stage
COPY --from=galib-builder /build/galib247 /build/galib247

# Copy source files from prog directory
COPY prog/src/main/cpp/ /build/sboxEvolver/prog/src/main/cpp/
COPY prog/makefile /build/sboxEvolver/prog/

WORKDIR /build/sboxEvolver/prog

# Patch Makefile for modern environment:
# 1. Change g++-4.4 to g++ (use default compiler)
# 2. Change -march=k8-sse3 to -march=native (optimize for build machine)
# 3. Change Python 2.5 include path to Python 2.7
# 4. Fix GAlib path to match Docker directory structure
RUN sed -i 's|CXX = g++-4.4|CXX = g++|g' makefile && \
    sed -i 's|-march=k8-sse3|-march=native|g' makefile && \
    sed -i 's|-I/usr/local/include/python2.5|-I/usr/include/python2.7|g' makefile && \
    sed -i 's|-I../galib247/|-I../../galib247|g' makefile && \
    sed -i 's|-L../galib247/ga|-L../../galib247/ga|g' makefile

# Patch project's template code for modern C++ (same issue as GAlib)
RUN sed -i 's/\tinitializer(/\tthis->initializer(/g' src/main/cpp/boxes.h && \
    sed -i 's/\tmutator(/\tthis->mutator(/g' src/main/cpp/boxes.h && \
    sed -i 's/\tcomparator(/\tthis->comparator(/g' src/main/cpp/boxes.h && \
    sed -i 's/\tcrossover(/\tthis->crossover(/g' src/main/cpp/boxes.h

# Build the shared library
RUN make

# ============================================
# Stage 3: Common Python environment
# ============================================
FROM python:2.7-slim AS python-base

# Python 2.7-slim is based on Debian Buster (archived) - update sources
RUN sed -i 's|http://deb.debian.org|http://archive.debian.org|g' /etc/apt/sources.list && \
    sed -i 's|http://security.debian.org|http://archive.debian.org|g' /etc/apt/sources.list && \
    sed -i '/buster-updates/d' /etc/apt/sources.list

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for experiments
RUN pip install --no-cache-dir \
    numpy==1.16.6 \
    matplotlib==2.2.5

# Set matplotlib to use non-interactive backend (no GUI required)
ENV MPLBACKEND=Agg

# ============================================
# Stage 4: Run tests
# ============================================
FROM python-base AS tester

# Install testing libraries (pytest < 5 and assertpy < 1 for Python 2.7 support)
RUN pip install --no-cache-dir "pytest<5" "assertpy<1"

WORKDIR /build/sboxEvolver

# Copy compiled library from builder stage to the Python directory
COPY --from=sbox-builder /build/sboxEvolver/prog/libsboxevolution.so /build/sboxEvolver/prog/src/test/python/

# Copy test files
COPY prog/src/test/python/ /build/sboxEvolver/prog/src/test/python/
COPY prog/src/main/python/ /build/sboxEvolver/prog/src/test/python/

# Run tests with pytest
RUN python -m pytest prog/src/test/python

# ============================================
# Stage 5: Runtime with artifacts
# ============================================
FROM python-base AS runtime

WORKDIR /app

# Copy compiled library from tester stage (ensures tests ran successfully)
COPY --from=tester /build/sboxEvolver/prog/src/test/python/libsboxevolution.so /app/

# Copy compiled library to artifacts directory for extraction
COPY --from=tester /build/sboxEvolver/prog/src/test/python/libsboxevolution.so /artifacts/

# Copy Python wrapper and experiments
COPY prog/src/main/python/ /app/

# Create output directory
RUN mkdir -p /app/outputs

# Environment variable for experiment number (default: 1)
ENV EXPERIMENT_NUM=1

# Run experiments.py with the experiment number argument
CMD python experiments.py ${EXPERIMENT_NUM}
