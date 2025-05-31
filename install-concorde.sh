#!/usr/bin/env bash
#
# install-concorde.sh
#
# This script builds only the Concorde command-line executable
# and installs it into /usr/local/bin (or a custom prefix) on a
# Debian-based system (e.g. Raspberry Pi OS).  It does NOT install
# Concorde’s libraries or headers—only the CLI binary itself.
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
BIN_DIR="$PREFIX/bin"

echo "Installing Concorde CLI into: $BIN_DIR"
echo

# ----------------------------------------------------------------------------–
# 1) Ensure build prerequisites are installed
# ----------------------------------------------------------------------------–
echo "==> Checking for required build tools..."

command -v gcc >/dev/null 2>&1 || {
  echo "Error: gcc not found. Please install 'build-essential' first.";
  exit 1;
}

command -v make >/dev/null 2>&1 || {
  echo "Error: make not found. Please install 'build-essential' first.";
  exit 1;
}

command -v git >/dev/null 2>&1 || {
  echo "Error: git not found. Please install git.";
  exit 1;
}

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

echo

# ----------------------------------------------------------------------------–
# 2) Clone or update the Concorde repository
# ----------------------------------------------------------------------------–
if [ ! -d concorde ]; then
  echo "==> Cloning Concorde repository..."
  git clone https://github.com/matthelb/concorde.git
else
  echo "==> 'concorde' directory already exists. Pulling latest changes..."
  cd concorde
  git fetch origin
  git checkout master
  git pull
  cd ..
fi
echo

# ----------------------------------------------------------------------------–
# 3) Enter the Concorde directory and prepare build
# ----------------------------------------------------------------------------–
cd concorde

# Ensure configure and Autotools helper scripts are executable
chmod +x configure
find . -type f \( -name "config.guess" -o -name "config.sub" \) -exec chmod +x {} \;

# ----------------------------------------------------------------------------–
# 4) Configure Concorde (top‐level)
# ----------------------------------------------------------------------------–
echo "==> Configuring Concorde (prefix=$PREFIX)..."
./configure --prefix="$PREFIX"
echo

# ----------------------------------------------------------------------------–
# 5) Build Concorde (full 'make', not a single target)
# ----------------------------------------------------------------------------–
echo "==> Building Concorde executable (make -j)..."
CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
make -j"$CORES"
echo

# ----------------------------------------------------------------------------–
# 6) Locate the built 'concorde' binary
# ----------------------------------------------------------------------------–
echo "==> Locating the 'concorde' CLI binary after build..."

# Common places where 'concorde' may appear:
if [ -x utilities/concorde/concorde ]; then
  BUILT_BIN="utilities/concorde/concorde"
elif [ -x concorde ]; then
  BUILT_BIN="concorde"
elif [ -x TSP/concorde ]; then
  BUILT_BIN="TSP/concorde"
else
  # As a fallback, try to find any executable named 'concorde' in subdirs
  FOUND=$(find . -type f -name concorde -perm /u+x 2>/dev/null | head -n 1)
  if [ -n "$FOUND" ]; then
    BUILT_BIN="$FOUND"
  else
    echo "Error: Cannot locate the built 'concorde' executable."
    exit 1
  fi
fi

echo "Found Concorde binary at: $BUILT_BIN"
echo

# ----------------------------------------------------------------------------–
# 7) Install the binary into $PREFIX/bin
# ----------------------------------------------------------------------------–
echo "==> Installing 'concorde' to $BIN_DIR"
mkdir -p "$BIN_DIR"
sudo cp "$BUILT_BIN" "$BIN_DIR/concorde"
sudo chmod 755 "$BIN_DIR/concorde"

echo
echo "Concorde CLI installed to: $BIN_DIR/concorde"
echo "You can verify by running: concorde -h"
echo

# ----------------------------------------------------------------------------–
# 8) (Optional) Clean up the source tree
# ----------------------------------------------------------------------------–
# If you want to remove the Concorde source directory after installation,
# uncomment the following lines:
#
# cd ..
# rm -rf concorde
#
# ----------------------------------------------------------------------------–

cd ..
