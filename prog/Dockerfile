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
FROM python:3.13-slim AS sbox-builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libgomp1 \
    python3.13-dev \
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
# 3. Change Python 2.5 include path to Python 3.13
# 4. Fix GAlib path to match Docker directory structure
# 5. Add Python 3.13 library linkage for Python C API functions
RUN sed -i 's|CXX = g++-4.4|CXX = g++|g' makefile && \
    sed -i 's|-march=k8-sse3|-march=native|g' makefile && \
    sed -i 's|-I/usr/local/include/python2.5|-I/usr/include/python3.13|g' makefile && \
    sed -i 's|-I../galib247/|-I../../galib247|g' makefile && \
    sed -i 's|-L../galib247/ga|-L../../galib247/ga|g' makefile && \
    sed -i 's|CXX_LIBS=-lga -lm|CXX_LIBS=-lga -lm -lpython3.13|g' makefile

# Patch project's template code for modern C++ (same issue as GAlib)
RUN sed -i 's/\tinitializer(/\tthis->initializer(/g' src/main/cpp/boxes.h && \
    sed -i 's/\tmutator(/\tthis->mutator(/g' src/main/cpp/boxes.h && \
    sed -i 's/\tcomparator(/\tthis->comparator(/g' src/main/cpp/boxes.h && \
    sed -i 's/\tcrossover(/\tthis->crossover(/g' src/main/cpp/boxes.h

# Build the shared library
RUN make

# ============================================
# Create Debian package for libsboxevolution.so
# ============================================

# Create package directory structure
RUN mkdir -p /build/sboxEvolver/prog/debian-package/DEBIAN && \
    mkdir -p /build/sboxEvolver/prog/debian-package/usr/lib

# Copy compiled library to package directory
RUN cp /build/sboxEvolver/prog/libsboxevolution.so \
       /build/sboxEvolver/prog/debian-package/usr/lib/

# Copy Debian package metadata files
COPY prog/debian/control /build/sboxEvolver/prog/debian-package/DEBIAN/control
COPY prog/debian/postinst /build/sboxEvolver/prog/debian-package/DEBIAN/postinst

# Make postinst executable
RUN chmod 755 /build/sboxEvolver/prog/debian-package/DEBIAN/postinst

# Build Debian package
RUN dpkg-deb --build /build/sboxEvolver/prog/debian-package \
              /build/libsboxevolution_2.0.0_amd64.deb

# Verify package was created and show its info
RUN ls -lh /build/libsboxevolution_2.0.0_amd64.deb && \
    echo "=== Package Info ===" && \
    dpkg-deb --info /build/libsboxevolution_2.0.0_amd64.deb && \
    echo "=== Package Contents ===" && \
    dpkg-deb --contents /build/libsboxevolution_2.0.0_amd64.deb

# ============================================
# Stage 3: Build Python dependencies (with numpy compilation)
# ============================================
FROM python:3.13-slim AS deps-builder

# Install build dependencies temporarily for numpy compilation
RUN apt-get update && apt-get install -y \
    gcc \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Install uv package manager
RUN pip install --no-cache-dir uv

# Set working directory for Python dependencies
WORKDIR /build/sboxEvolver/prog

# Copy pyproject.toml and build dependencies with numpy wheels
COPY prog/pyproject.toml .
RUN uv sync --no-dev

# ============================================
# Stage 4: Common Python runtime (without build tools)
# ============================================
FROM python:3.13-slim AS python-base

# Copy .deb package from sbox-builder stage
COPY --from=sbox-builder /build/libsboxevolution_2.0.0_amd64.deb /tmp/

# Install runtime dependencies AND the library package
# The .deb package will install libsboxevolution.so to /usr/lib/
RUN apt-get update && apt-get install -y \
    libgomp1 \
    /tmp/libsboxevolution_2.0.0_amd64.deb \
    && rm -rf /var/lib/apt/lists/* /tmp/libsboxevolution_2.0.0_amd64.deb

# Verify library is installed in system path
RUN echo "=== Verifying library installation ===" && \
    ls -l /usr/lib/libsboxevolution.so && \
    ldconfig -p | grep libsboxevolution && \
    echo "✓ Library successfully installed to /usr/lib/"

# Install uv package manager
RUN pip install --no-cache-dir uv

# Set working directory
WORKDIR /build/sboxEvolver/prog

# Copy built virtual environment from deps-builder
COPY --from=deps-builder /build/sboxEvolver/prog/.venv /build/sboxEvolver/prog/.venv
COPY --from=deps-builder /build/sboxEvolver/prog/pyproject.toml /build/sboxEvolver/prog/pyproject.toml

# Set matplotlib to use non-interactive backend (no GUI required)
ENV MPLBACKEND=Agg

# ============================================
# Stage 5: Run tests
# ============================================
FROM python-base AS tester

WORKDIR /build/sboxEvolver/prog

# Copy pyproject.toml and install dev dependencies (includes pytest and assertpy)
COPY prog/pyproject.toml .
RUN uv sync  # Includes dev dependencies

# Copy compiled library from builder stage
COPY --from=sbox-builder /build/sboxEvolver/prog/libsboxevolution.so ./

# Copy source files (main and test)
COPY prog/src/ ./src/

# Set PYTHONPATH so tests can import from src/main/python
ENV PYTHONPATH=/build/sboxEvolver/prog/src/main/python

# Run tests with pytest via uv
RUN uv run pytest src/test/python

# ============================================
# Stage 6: Runtime with artifacts
# ============================================
FROM python-base AS runtime

WORKDIR /app

# Copy Debian package from sbox-builder for host extraction
COPY --from=sbox-builder /build/libsboxevolution_2.0.0_amd64.deb /artifacts/

# Copy compiled library from tester stage for host extraction
# (Ensures tests passed before extraction - runtime depends on tester)
COPY --from=tester /build/sboxEvolver/prog/libsboxevolution.so /artifacts/

# Note: Library is already installed in /usr/lib/ via the .deb package in python-base
# Verify library is accessible via system path
RUN echo "=== Verifying runtime library access ===" && \
    ldconfig -p | grep libsboxevolution && \
    ls -l /usr/lib/libsboxevolution.so && \
    echo "✓ Library accessible at /usr/lib/libsboxevolution.so"

# Copy Python wrapper and experiments
COPY prog/src/main/python/ /app/

# Copy the virtual environment from python-base to /app
COPY --from=python-base /build/sboxEvolver/prog/.venv /app/.venv

# Create output directory
RUN mkdir -p /app/outputs

# Environment variable for experiment number (default: 1)
ENV EXPERIMENT_NUM=1

# Set VIRTUAL_ENV and PATH to use the venv
ENV VIRTUAL_ENV=/app/.venv
ENV PATH="/app/.venv/bin:$PATH"

# Run experiments.py with the experiment number argument
CMD python experiments.py ${EXPERIMENT_NUM}
