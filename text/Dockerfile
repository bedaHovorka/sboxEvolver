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
#      LaTeX thesis compilation with Czech language tools
#

FROM debian:bookworm-slim

# Install TeX Live and Czech language tools
RUN apt-get update && apt-get install -y \
    # Core LaTeX packages
    texlive-latex-base \
    texlive-latex-extra \
    texlive-bibtex-extra \
    texlive-fonts-recommended \
    texlive-font-utils \
    texlive-science \
    # Czech language support
    texlive-lang-czechslovak \
    texlive-binaries \
    cstocs \
    # Image conversion tools
    inkscape \
    dia \
    # Build tool
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /thesis

# Copy entire text directory (includes img/ subdirectory)
COPY text/ /thesis/

# Patch img/Makefile to ignore errors on individual image conversions (add - prefix)
RUN sed -i '29s/^/\t-/' img/Makefile

# Build the thesis (4-pass LaTeX compilation with bibliography)
# Ignore errors from image conversion issues
RUN make || true
RUN make

# Default command: copy PDF to mounted volume and signal completion
CMD ["sh", "-c", "cp diplomka.pdf /artifacts/ && echo 'Thesis compiled: diplomka.pdf copied to /artifacts/'"]