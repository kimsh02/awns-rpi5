#!/usr/bin/env bash
#
# install-concorde.sh
#
# Clone, configure, build, and install Concorde TSP solver into /usr/local (or another prefix).
# This version is intended to run on Raspberry Pi OS (Debian-based Linux).
#
# Usage:
#   chmod +x install-concorde.sh
#   ./install-concorde.sh [<install_prefix>]
#
# Example:
#   ./install-concorde.sh
#   ./install-concorde.sh "$HOME/.local"
#

set -euo pipefail

PREFIX="${1:-/usr/local}"
echo "==> Installing Concorde into prefix: ${PREFIX}"
echo

# 1) Ensure required tools are present
if ! command -v gcc >/dev/null 2>&1; then
  echo "Error: gcc not found. Please install build-essential."
  exit 1
fi

if ! command -v make >/dev/null 2>&1; then
  echo "Error: make not found. Please install build-essential."
  exit 1
fi

# 2) Clone or update the Concorde repository
if [ ! -d concorde ]; then
  echo "[1/4] Cloning Concorde repository..."
  git clone https://github.com/matthelb/concorde.git
  echo "[1/4] Clone complete."
else
  echo "[1/4] 'concorde' directory exists. Updating to latest master..."
  cd concorde
  git fetch origin
  git checkout master
  git pull
  cd ..
  echo "[1/4] Update complete."
fi
echo

# 3) Enter the Concorde directory and make scripts executable
echo "[2/4] Entering 'concorde' directory..."
cd concorde
echo "[2/4] Now in: $(pwd)"
echo

echo "[2/4] Ensuring configure and related scripts are executable..."
chmod +x configure
find . -type f \( -name "config.guess" -o -name "config.sub" \) -exec chmod +x {} \;
echo "[2/4] chmod step done."
echo

# 4) Run configure with GCC on Raspberry Pi OS
echo "[3/4] Running CC=gcc ./configure --prefix=${PREFIX} ..."
export CC=gcc
if ! ./configure --prefix="${PREFIX}"; then
  echo "Error: ./configure failed on Linux. Aborting."
  exit 1
fi
echo "[3/4] ./configure succeeded."
echo

# 5) Build and install
echo "[4/4] Building Concorde (make -j) and installing into ${PREFIX}..."
CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
echo "[4/4] Using ${CORES} core(s) for build."
make -j"${CORES}"

if [ -w "${PREFIX}" ]; then
  make install
else
  echo "[4/4] Need sudo to install into ${PREFIX} – prompting for password."
  sudo make install
fi

echo "[4/4] Installation complete."
echo
echo "Concorde headers: ${PREFIX}/include/tsp.h"
echo "Concorde library: ${PREFIX}/lib/libconcorde.a"
echo

# Return to project root
cd ..
