#!/usr/bin/env bash
#
# install-concorde-macos-gcc13-with-qsopt.sh
#
# Builds and installs QSopt_ex LP solver and Concorde TSP solver
# using gcc-13 on macOS (Apple Silicon).
#
set -euo pipefail

# 0) Parse args / vars
PREFIX="${1:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib"
INCLUDE_DIR="$PREFIX/include"
CORES="$(sysctl -n hw.ncpu 2>/dev/null || echo 1)"

echo "Installing Concorde + QSopt_ex → prefix: $PREFIX"

echo "==> Ensure build tools"
for tool in git make gcc-13 g++-13 brew; do
  command -v "$tool" >/dev/null 2>&1 || { echo "Error: $tool not found."; exit 1; }
done

echo "==> Homebrew packages"
brew update
brew install gmp zlib bzip2 libtool automake autoconf \
             openblas lapack readline flex bison pkg-config wget gcc@13

echo "==> QSopt_ex clone/update"
if [ ! -d qsopt-ex ]; then
  git clone https://github.com/jonls/qsopt-ex.git
else
  cd qsopt-ex && git pull && cd ..
fi

# 3) Build QSopt_ex
echo "==> Build QSopt_ex"
cd qsopt-ex
./bootstrap
mkdir -p build && cd build
GMP_PREFIX="$(brew --prefix gmp)"
export CPPFLAGS="-I$GMP_PREFIX/include"
export LDFLAGS="-L$GMP_PREFIX/lib"
export PKG_CONFIG_PATH="$GMP_PREFIX/lib/pkgconfig"
CC=gcc-13 CXX=g++-13 ../configure --prefix="$PREFIX"
make -j"$CORES"
sudo make install
# create symlink for Concorde to find qsopt.a
sudo ln -sf "$LIB_DIR/libqsopt_ex.a" "$PREFIX/qsopt.a"
cd ../..

echo "==> Concorde clone/update"
if [ ! -d concorde ]; then
  git clone https://github.com/matthelb/concorde.git
else
  cd concorde && git fetch origin && git checkout master && git pull && cd ..
fi

echo "==> Patch headers"
cd concorde
# use lowercase include dir
HDR_DIR=include
if [ -f "$HDR_DIR/config.h" ]; then
  sed -i.bak "/^#define u_char unsigned char/ s|^|// |" "$HDR_DIR/config.h"
fi
if [ -f "$HDR_DIR/machdefs.h" ]; then
  sed -i.bak -E "/gethostname *\( *char/,/;/ s|^|// |" "$HDR_DIR/machdefs.h"
fi

echo "==> Refresh autotools scripts"
CONFIG_GUESS_URL="https://git.savannah.gnu.org/cgit/config.git/plain/config.guess"
CONFIG_SUB_URL="https://git.savannah.gnu.org/cgit/config.git/plain/config.sub"
# root
curl -fsSL "$CONFIG_GUESS_URL" > config.guess && chmod +x config.guess
curl -fsSL "$CONFIG_SUB_URL"  > config.sub  && chmod +x config.sub
# propagate: copy into any directory with a configure script
find . -type f -name configure | while read cfg; do
  dir=$(dirname "$cfg")
  cp config.guess "$dir"/config.guess
  cp config.sub  "$dir"/config.sub
  chmod +x "$dir"/config.guess "$dir"/config.sub
done

echo "==> Configure Concorde"
chmod +x configure
export CC=gcc-13 CXX=g++-13
export PKG_CONFIG_PATH="$(brew --prefix openblas)/lib/pkgconfig:$(brew --prefix lapack)/lib/pkgconfig"
./configure \
  --prefix="$PREFIX" \
  --with-blas="-L$(brew --prefix openblas)/lib -lopenblas" \
  --with-lapack="-L$(brew --prefix lapack)/lib -llapack" \
  --with-qsopt="$PREFIX/qsopt.a"

echo "==> Build Concorde"
make -j"$CORES"

echo "==> Install Concorde"
sudo mkdir -p "$BIN_DIR" "$LIB_DIR" "$INCLUDE_DIR"
sudo install -m 755 utilities/concorde/concorde "$BIN_DIR/concorde"
sudo install -m 644 concorde.a "$LIB_DIR/libconcorde.a"
# headers
sudo cp -r include/* "$INCLUDE_DIR/"
[ -f concorde.h ] && sudo install -m 644 concorde.h "$INCLUDE_DIR/concorde.h"

echo "Done!" cd ..
exit 0
