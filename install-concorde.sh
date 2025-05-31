#!/usr/bin/env bash
#
# install_concorde.sh
#
# Clone and install Concorde TSP solver into /usr/local (or another prefix).
# Usage: ./install-concorde.sh [<install_prefix>]
#

set -e
set -o pipefail

# Default prefix is /usr/local; override by passing an argument.
PREFIX="${1:-/usr/local}"

echo "==> Installing Concorde into prefix: ${PREFIX}"

# 1) Clone the Concorde repository (if not already present)
if [ ! -d concorde ]; then
  echo "Cloning Concorde repository..."
  git clone https://github.com/matthelb/concorde.git
else
  echo "Concorde directory exists; pulling latest changes..."
  cd concorde
  git fetch origin
  git checkout master
  git pull
  cd ..
fi

# 2) Enter the concorde directory
cd concorde

# 3) Run configure with the PREFIX
echo "Running ./configure --prefix=${PREFIX} ..."
./configure --prefix="${PREFIX}"

# 4) Build with all available cores
CORES=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
echo "Building Concorde using ${CORES} cores..."
make -j"${CORES}"

# 5) Install (requires sudo if /usr/local is root‐owned)
echo "Installing Concorde into ${PREFIX} ..."
if [ -w "${PREFIX}" ]; then
  make install
else
  echo "sudo is required for installation into ${PREFIX}"
  sudo make install
fi

echo "Concorde install complete."
