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
CORES="$(nproc || echo 1)"

echo "Prefix:       $PREFIX"
echo "QSopt-ex at:  $QSOPT_PREFIX"
echo

# 1) System prerequisites
echo "==> Installing apt packages…"
apt-get update
apt-get install -y \
  build-essential git autoconf automake libtool pkg-config \
  gcc g++ make wget curl \
  libgmp-dev liblapack-dev libblas-dev libreadline-dev \
  libbz2-dev zlib1g-dev flex bison
echo

# 2) Clone & build QSopt-ex
if [ ! -d qsopt-ex ]; then
  echo "==> Cloning QSopt-ex…"
  git clone https://github.com/jonls/qsopt-ex.git
else
  echo "==> Updating QSopt-ex…"
  cd qsopt-ex && git pull && cd ..
fi
echo "==> Building QSopt-ex…"
cd qsopt-ex
./bootstrap
mkdir -p build && cd build
../configure --prefix="$QSOPT_PREFIX"
make -j"$CORES"
make install
cd ../..
echo

# 3) Clone Concorde and patch machdefs.h
if [ ! -d concorde ]; then
  echo "==> Cloning Concorde…"
  git clone https://github.com/matthelb/concorde.git
else
  echo "==> Updating Concorde…"
  cd concorde && git fetch origin && git checkout master && git pull && cd ..
fi

echo "==> Patching gethostname in machdefs.h…"
cd concorde

# detect include directory case
HDR_DIR=INCLUDE
[ -d include ] && HDR_DIR=include

# comment out gethostname prototype
sed -i.bak -E \
  's|^[[:space:]]*gethostname *\(.*\);|// &|' \
  "$HDR_DIR/machdefs.h"
echo

# 4) Refresh autotools support
echo "==> Refreshing config.guess/config.sub…"
curl -fsSL https://git.savannah.gnu.org/cgit/config.git/plain/config.guess \
     -o config.guess
curl -fsSL https://git.savannah.gnu.org/cgit/config.git/plain/config.sub \
     -o config.sub
chmod +x configure config.guess config.sub
find . -type f \( -name config.guess -o -name config.sub \) -exec chmod +x {} \;
echo

# 5) Configure with QSopt-ex
echo "==> Configuring Concorde with QSopt-ex…"
export CC=gcc
export CXX=g++
./configure \
  --prefix="$PREFIX" \
  --with-qsopt="$QSOPT_PREFIX" \
  --with-blas="" --with-lapack=""
echo

# 6) Patch generated config.h
echo "==> Patching u_char in generated config.h…"
sed -i.bak \
  's|^#define u_char unsigned char|// &|' \
  "$HDR_DIR/config.h"
echo

# 7) Build Concorde
echo "==> Building Concorde…"
make -j"$CORES"
echo

# 8) Install
echo "==> Installing to $PREFIX…"
mkdir -p "$BIN_DIR" "$LIB_DIR" "$INCLUDE_DIR"

# CLI
install -m755 utilities/concorde/concorde "$BIN_DIR/concorde"

# static library
install -m644 concorde.a "$LIB_DIR/libconcorde.a"

# headers
cp -r "$HDR_DIR"/*.h "$INCLUDE_DIR/"
[ -f concorde.h ] && install -m644 concorde.h "$INCLUDE_DIR/"

echo
echo "✔ Done."
echo "  - concorde → $BIN_DIR/concorde"
echo "  - libconcorde.a → $LIB_DIR/libconcorde.a"
echo "  - headers → $INCLUDE_DIR/"
