#!/usr/bin/env bash
# install-concorde-macos-gcc13-with-qsopt.sh
# Builds and installs QSopt_ex LP solver and Concorde TSP solver
# using gcc-13 on macOS (Apple Silicon).
set -euo pipefail

# 0) Parse args
PREFIX="${1:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib"
INCLUDE_DIR="$PREFIX/include"
CORES="$(sysctl -n hw.ncpu 2>/dev/null || echo 1)"

echo "Installing Concorde + QSopt_ex → \$PREFIX"

# 1) Prereqs
for tool in git make gcc-13 g++-13 brew; do
  command -v "$tool" >/dev/null 2>&1 || { echo "Error: $tool not found."; exit 1; }
done

# 2) Homebrew deps
brew update
brew install gmp zlib bzip2 libtool automake autoconf \
             openblas lapack readline flex bison pkg-config wget gcc@13

# 3) QSopt_ex
if [ ! -d qsopt-ex ]; then
  git clone https://github.com/jonls/qsopt-ex.git
else
  cd qsopt-ex && git pull && cd ..
fi
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
# expose qsopt headers & library for Concorde
sudo ln -sf "$PREFIX/lib/libqsopt_ex.a" "$PREFIX/qsopt.a"
sudo ln -sf "$PREFIX/include/qsopt_ex/QSopt_ex.h" "$PREFIX/include/qsopt.h"
cd ../..

echo "↪ Linking QSopt_ex into the places Concorde expects…"
PREFIX=/usr/local

# 1) Header symlink so <qsopt.h> really lives at /usr/local/include/qsopt.h
sudo ln -sf "${PREFIX}/include/qsopt_ex/QSopt_ex.h" \
      "${PREFIX}/include/qsopt.h"

# 2) Library symlink so -lqsopt (i.e. libqsopt.a) picks up libqsopt_ex.a
sudo ln -sf "${PREFIX}/lib/libqsopt_ex.a" \
      "${PREFIX}/lib/qsopt.a"
sudo ln -sf "${PREFIX}/lib/libqsopt_ex.dylib" \
      "${PREFIX}/lib/qsopt.dylib"

# 3) Make sure the compiler actually searches /usr/local/include and /usr/local/lib
export CFLAGS="-I${PREFIX}/include"
export LDFLAGS="-L${PREFIX}/lib"

# 4) Concorde
if [ ! -d concorde ]; then
  git clone https://github.com/matthelb/concorde.git
else
  cd concorde && git fetch origin && git checkout master && git pull && cd ..
fi
echo "Patching headers..."
cd concorde
HDR=include
[ -f "$HDR/config.h" ] && sed -i.bak "/^#define u_char unsigned char/ s|^|// |" "$HDR/config.h"
[ -f "$HDR/machdefs.h" ] && sed -i.bak -E "/gethostname *\( *char/,/;/ s|^|// |" "$HDR/machdefs.h"

echo "Updating autotools scripts..."
CONFIG_GUESS_URL="https://git.savannah.gnu.org/cgit/config.git/plain/config.guess"
CONFIG_SUB_URL="https://git.savannah.gnu.org/cgit/config.git/plain/config.sub"
curl -fsSL "$CONFIG_GUESS_URL" > config.guess && chmod +x config.guess
curl -fsSL "$CONFIG_SUB_URL"  > config.sub  && chmod +x config.sub
find . -mindepth 2 -type f -name configure | while read -r cfg; do
  d=$(dirname "$cfg")
  cp config.guess "$d"/config.guess
  cp config.sub  "$d"/config.sub
  chmod +x "$d"/config.guess "$d"/config.sub
done

# 5) Configure Concorde
chmod +x configure
export CC=gcc-13 CXX=g++-13
# include QSopt_ex headers
export CPPFLAGS="-I$PREFIX/include/qsopt_ex $CPPFLAGS"
export PKG_CONFIG_PATH="$(brew --prefix openblas)/lib/pkgconfig:$(brew --prefix lapack)/lib/pkgconfig"
./configure \
  --prefix="$PREFIX" \
  --with-blas="-L$(brew --prefix openblas)/lib -lopenblas" \
  --with-lapack="-L$(brew --prefix lapack)/lib -llapack" \
  --with-qsopt="$PREFIX/qsopt.a"

echo "Building Concorde..."
make -j"$CORES"

echo "Installing Concorde..."
sudo mkdir -p "$BIN_DIR" "$LIB_DIR" "$INCLUDE_DIR"
sudo install -m755 utilities/concorde/concorde "$BIN_DIR/concorde"
sudo install -m644 concorde.a       "$LIB_DIR/libconcorde.a"
sudo cp -r include/* "$INCLUDE_DIR/"
[ -f concorde.h ] && sudo install -m644 concorde.h "$INCLUDE_DIR/concorde.h"
cd ..
echo "Done!"
exit 0
