#!/usr/bin/env bash
#
# install-concorde-raspberrypi.sh
#
# Builds and installs QSopt-ex LP solver and Concorde TSP solver
# on Raspberry Pi OS (ARM64). Installs into <PREFIX> (/usr/local by default).
#
# Usage:
#   chmod +x install-concorde-raspberrypi.sh
#   sudo ./install-concorde-raspberrypi.sh [<install_prefix>]
#

set -euo pipefail

PREFIX="${1:-/usr/local}"
QSOPT_PREFIX="$PREFIX/qsoptex"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib"
INCLUDE_DIR="$PREFIX/include"

echo "Prefix:       $PREFIX"
echo "QSopt-ex at:  $QSOPT_PREFIX"
echo

CORES="$(nproc || echo 1)"

# 1) Install system prerequisites
echo "==> Installing apt packages..."
apt-get update
apt-get install -y \
    build-essential git autoconf automake libtool pkg-config \
    gcc g++ make wget curl \
    libgmp-dev liblapack-dev libblas-dev libreadline-dev \
    libbz2-dev zlib1g-dev flex bison
echo

# 2) Clone & build QSopt-ex
if [ ! -d qsopt-ex ]; then
  echo "==> Cloning QSopt-ex..."
  git clone https://github.com/jonls/qsopt-ex.git
else
  echo "==> Updating QSopt-ex..."
  cd qsopt-ex && git pull && cd ..
fi

echo "==> Building QSopt-ex..."
cd qsopt-ex
./bootstrap
mkdir -p build && cd build
../configure --prefix="$QSOPT_PREFIX"
make -j"$CORES"
make install
cd ../..
echo

# 3) Clone & patch Concorde
if [ ! -d concorde ]; then
  echo "==> Cloning Concorde..."
  git clone https://github.com/matthelb/concorde.git
else
  echo "==> Updating Concorde..."
  cd concorde && git fetch origin && git checkout master && git pull && cd ..
fi

echo "==> Applying u_char/gethostname patches..."
cd concorde

# comment out u_char in config.h
if grep -q '^#define u_char unsigned char' INCLUDE/config.h; then
  sed -i.bak 's|^#define u_char unsigned char|// #define u_char unsigned char|' \
    INCLUDE/config.h
fi

# comment out gethostname prototype
if grep -q '^[[:space:]]*gethostname *(char \*, *int);' INCLUDE/machdefs.h; then
  sed -i.bak -E 's|^[[:space:]]*(gethostname *\( *char \*, *int *\);)|// \1|' \
    INCLUDE/machdefs.h
fi

# refresh config.guess/config.sub for ARM
echo "==> Refreshing config.guess/config.sub..."
curl -fsSL https://git.savannah.gnu.org/cgit/config.git/plain/config.guess \
     -o config.guess
curl -fsSL https://git.savannah.gnu.org/cgit/config.git/plain/config.sub \
     -o config.sub
chmod +x config.guess config.sub
find . -type f \( -name config.guess -o -name config.sub \) -exec chmod +x {} \;

cd ..
echo

# 4) Configure Concorde with QSopt-ex
echo "==> Configuring Concorde with QSopt-ex..."
cd concorde
export CC=gcc
export CXX=g++
# tell configure where to find qsopt library & headers
./configure --prefix="$PREFIX" \
            --with-qsopt="$QSOPT_PREFIX" \
            --with-blas="" --with-lapack=""
echo

# 5) Build & install Concorde
echo "==> Building Concorde..."
make -j"$CORES"
echo

echo "==> Installing Concorde CLI, lib, headers..."
mkdir -p "$BIN_DIR" "$LIB_DIR" "$INCLUDE_DIR"

# CLI
install -m 755 utilities/concorde/concorde "$BIN_DIR/concorde"

# static library
install -m 644 concorde.a "$LIB_DIR/libconcorde.a"

# headers
cp -r INCLUDE/* "$INCLUDE_DIR/"
[ -f concorde.h ] && install -m 644 concorde.h "$INCLUDE_DIR/"

echo
echo "✔ Done."
echo "  - Concorde CLI: $BIN_DIR/concorde"
echo "  - lib:           $LIB_DIR/libconcorde.a"
echo "  - headers:       $INCLUDE_DIR/*.h"
