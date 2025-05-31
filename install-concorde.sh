#!/usr/bin/env bash
#
# install-concorde.sh
#
# This script builds only the Concorde command-line executable
# and installs it into /usr/local/bin on a Debian-based system
# (e.g. Raspberry Pi OS).  It does NOT install Concorde’s libraries
# or headers—only the CLI binary.
#
# Usage:
#   chmod +x install-concorde.sh
#   ./install-concorde.sh
#
# Optional argument:
#   You may pass a custom install prefix (e.g. "$HOME/.local") as $1.
#   By default it installs into /usr/local.
#
# Example:
#   ./install-concorde.sh
#   ./install-concorde.sh "$HOME/.local"
#

set -euo pipefail

PREFIX="${1:-/usr/local}"
BIN_DIR="$PREFIX/bin"

echo "Installing Concorde CLI into: $BIN_DIR"
echo

# 1) Ensure build prerequisites are installed
echo "==> Checking for required build tools..."
command -v gcc >/dev/null 2>&1 || { echo "Error: gcc not found. Install build-essential."; exit 1; }
command -v make >/dev/null 2>&1 || { echo "Error: make not found. Install build-essential."; exit 1; }
command -v git >/dev/null 2>&1 || { echo "Error: git not found. Install git."; exit 1; }

# Optional but strongly recommended dependencies for Concorde
echo "==> Installing recommended libraries (libgmp-dev, liblapack-dev, libblas-dev, etc.)"
sudo apt-get update
sudo apt-get install -y \
  libgmp-dev \
  liblapack-dev \
  libblas-dev \
  libreadline-dev \
  libbz2-dev \
  flex \
  bison \
  pkg-config \
  build-essential \
  git

# 2) Clone Concorde repository (or update if it already exists)
if [ ! -d concorde ]; then
  echo "==> Cloning Concorde repository..."
  git clone https://github.com/matthelb/concorde.git
else
  echo "==> 'concorde' directory already exists. Updating to latest master..."
  cd concorde
  git fetch origin
  git checkout master
  git pull
  cd ..
fi
echo

# 3) Enter the Concorde directory and prepare build
cd concorde
chmod +x configure
find . -type f \( -name "config.guess" -o -name "config.sub" \) -exec chmod +x {} \;

# 4) Configure and build only the CLI
echo "==> Configuring Concorde (prefix=$PREFIX)..."
./configure --prefix="$PREFIX"

echo "==> Running make to build the Concorde executable..."
CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
make -j"$CORES" utilities/concorde/concorde

# 5) Install the binary into $PREFIX/bin
echo "==> Installing concorde to $BIN_DIR"
mkdir -p "$BIN_DIR"
sudo cp utilities/concorde/concorde "$BIN_DIR/concorde"
sudo chmod 755 "$BIN_DIR/concorde"

echo
echo "Concorde CLI has been installed to $BIN_DIR/concorde"
echo "You can now run: concorde -h"
echo

# 6) (Optional) Clean up build artifacts if desired
# Uncomment the following lines to remove the source tree:
# cd ..
# rm -rf concorde

cd ..
