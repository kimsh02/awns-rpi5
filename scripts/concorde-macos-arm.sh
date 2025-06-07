#!/usr/bin/env bash
#
# install-concorde-macos-gcc13.sh
#
# This script clones (or updates) the Concorde repository, applies small patches
# to avoid u_char / gethostname conflicts, fetches fresh config.guess/config.sub so
# that Apple Silicon (arm64-apple-darwin) is detected, builds Concorde’s static
# library and CLI using gcc-13, and installs them into:
#     <PREFIX>/bin/concorde
#     <PREFIX>/lib/libconcorde.a
#     <PREFIX>/include/    (all Concorde header files)
#
# Usage:
#   chmod +x install-concorde-macos-gcc13.sh
#   ./install-concorde-macos-gcc13.sh [<install_prefix>]
#
# If you omit <install_prefix>, it defaults to /usr/local. If you don’t have write
# permission under /usr/local, pass something like "$HOME/.local" instead.
#

set -euo pipefail

# ──────────────────────────────────────────────────────────────────────────────
# 0) Parse arguments / define variables
# ──────────────────────────────────────────────────────────────────────────────
PREFIX="${1:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib"
INCLUDE_DIR="$PREFIX/include"

echo "Installing Concorde (CLI, library, headers) on macOS (Apple Silicon) into:"
echo "  Prefix:    $PREFIX"
echo "  Bin dir:   $BIN_DIR"
echo "  Lib dir:   $LIB_DIR"
echo "  Include:   $INCLUDE_DIR"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 1) Verify & install prerequisites via Homebrew
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Checking for build tools (git, make, gcc-13)…"
command -v git     >/dev/null 2>&1 || { echo "Error: git not found. Install Xcode Command Line Tools."; exit 1; }
command -v make    >/dev/null 2>&1 || { echo "Error: make not found. Install Xcode Command Line Tools."; exit 1; }
command -v gcc-13  >/dev/null 2>&1 || { echo "Error: gcc-13 not found. Run: brew install gcc@13"; exit 1; }
command -v g++-13  >/dev/null 2>&1 || { echo "Error: g++-13 not found. Run: brew install gcc@13"; exit 1; }
echo

echo "==> Installing (or verifying) Homebrew packages…"
if ! command -v brew >/dev/null 2>&1; then
  echo "Error: Homebrew (brew) not found. Please install Homebrew first:"
  echo "  https://brew.sh"
  exit 1
fi

brew update
brew install gmp openblas lapack readline bzip2 flex bison pkg-config wget gcc@13
echo

# ──────────────────────────────────────────────────────────────────────────────
# 2) Clone or update the Concorde repository
# ──────────────────────────────────────────────────────────────────────────────
if [ ! -d concorde ]; then
  echo "==> Cloning Concorde repository…"
  git clone https://github.com/matthelb/concorde.git
else
  echo "==> 'concorde' directory already exists. Updating…"
  cd concorde
  git fetch origin
  git checkout master
  git pull
  cd ..
fi

echo

# ──────────────────────────────────────────────────────────────────────────────
# 3) Apply in‐place patches (avoid u_char / gethostname conflicts)
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Applying patches to avoid u_char/gethostname conflicts…"
cd concorde

# Determine whether headers live in uppercase "INCLUDE/" or lowercase "include/"
if [ -d "INCLUDE" ]; then
  HDR_DIR="INCLUDE"
elif [ -d "include" ]; then
  HDR_DIR="include"
else
  echo "Error: Cannot find Concorde’s include directory (INCLUDE/ or include/)."
  exit 1
fi

# 3a) In $HDR_DIR/config.h, comment out "#define u_char unsigned char"
if [ -f "$HDR_DIR/config.h" ] && grep -q '^#define u_char unsigned char' "$HDR_DIR/config.h"; then
  sed -i.bak \
    "s|^#define u_char unsigned char|// #define u_char unsigned char|" \
    "$HDR_DIR/config.h"
  echo "    • Patched: commented out u_char in $HDR_DIR/config.h"
else
  echo "    • (Skipped) u_char define not found in $HDR_DIR/config.h"
fi

# 3b) In $HDR_DIR/machdefs.h, comment out "gethostname(char *, int);"
if [ -f "$HDR_DIR/machdefs.h" ] && grep -q '^[[:space:]]*gethostname *(char \*, *int);' "$HDR_DIR/machdefs.h"; then
  sed -i.bak -E \
    "s|^[[:space:]]*(gethostname *\( *char \*, *int *\);)|// \1|" \
    "$HDR_DIR/machdefs.h"
  echo "    • Patched: commented out gethostname prototype in $HDR_DIR/machdefs.h"
else
  echo "    • (Skipped) gethostname prototype not found in $HDR_DIR/machdefs.h"
fi

echo

# ──────────────────────────────────────────────────────────────────────────────
# 4) Refresh config.guess/config.sub so Apple Silicon is detected
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Refreshing config.guess and config.sub (GNU autotools)…"
CONFIG_GUESS_URL="https://git.savannah.gnu.org/cgit/config.git/plain/config.guess"
CONFIG_SUB_URL="https://git.savannah.gnu.org/cgit/config.git/plain/config.sub"

# Remove any old copies
find . -type f \( -name "config.guess" -o -name "config.sub" \) -delete 2>/dev/null || true

# Download latest scripts at root
curl -fsSL "$CONFIG_GUESS_URL" > config.guess
curl -fsSL "$CONFIG_SUB_URL"  > config.sub
chmod +x config.guess config.sub

# Propagate them into subdirectories (if any nested configure is used)
find . -type d -exec sh -c '
  for sub in "$1"/config.*; do
    if [ -d "$sub" ]; then
      cp "$(dirname "$0")/config.guess" "$sub/config.guess"
      cp "$(dirname "$0")/config.sub"  "$sub/config.sub"
      chmod +x "$sub/config.guess" "$sub/config.sub"
    fi
  done
' sh config.guess {} \;

echo "    • config.guess and config.sub updated"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 5) Ensure configure is executable
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Ensuring 'configure' is executable…"
chmod +x configure
echo "    • 'configure', 'config.guess', and 'config.sub' are now executable"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 6) Configure Concorde (explicit BLAS/LAPACK flags)
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Running ./configure --prefix=$PREFIX with BLAS/LAPACK settings and gcc-13…"

# Force environment to use gcc-13/g++-13
export CC=gcc-13
export CXX=g++-13
export PKG_CONFIG_PATH="$(brew --prefix openblas)/lib/pkgconfig:$(brew --prefix lapack)/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

# Pass explicit BLAS/LAPACK to configure
./configure --prefix="$PREFIX" \
            --with-blas="-L$(brew --prefix openblas)/lib -lopenblas" \
            --with-lapack="-L$(brew --prefix lapack)/lib -llapack"

echo

# ──────────────────────────────────────────────────────────────────────────────
# 7) Build Concorde (static library & CLI)
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Compiling Concorde (this may take a minute)…"
CORES="$(sysctl -n hw.ncpu 2>/dev/null || echo 1)"
make -j"$CORES"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 8) Locate the built 'concorde' CLI and static library
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Locating built Concorde artifacts…"

# 8a) CLI binary
if [ -x utilities/concorde/concorde ]; then
  BUILT_CLI="utilities/concorde/concorde"
elif [ -x TSP/concorde ]; then
  BUILT_CLI="TSP/concorde"
elif [ -x ./concorde ]; then
  BUILT_CLI="./concorde"
else
  FOUND=$(find . -type f -name concorde -perm /u+x 2>/dev/null | head -n 1 || echo "")
  BUILT_CLI="$FOUND"
fi

if [ -z "$BUILT_CLI" ] || [ ! -x "$BUILT_CLI" ]; then
  echo "Error: Could not find the 'concorde' executable after build."
  exit 1
fi
echo "    • Found CLI at: $BUILT_CLI"

# 8b) Static library
if [ -f concorde.a ]; then
  BUILT_LIB="concorde.a"
elif [ -f libconcorde.a ]; then
  BUILT_LIB="libconcorde.a"
else
  FOUND_LIB=$(find . -type f -name "concorde.a" 2>/dev/null | head -n 1 || echo "")
  BUILT_LIB="$FOUND_LIB"
fi

if [ -z "$BUILT_LIB" ] || [ ! -f "$BUILT_LIB" ]; then
  echo "Error: Could not find concorde.a (static library)."
  exit 1
fi
echo "    • Found static library at: $BUILT_LIB"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 9) Install CLI, library, and headers into $PREFIX
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Installing Concorde components into prefix: $PREFIX"
echo "    • Creating directories: $BIN_DIR, $LIB_DIR, $INCLUDE_DIR"
mkdir -p "$BIN_DIR" "$LIB_DIR" "$INCLUDE_DIR"

# 9a) Install CLI
echo "    • Installing CLI → $BIN_DIR/concorde"
sudo cp "$BUILT_CLI" "$BIN_DIR/concorde"
sudo chmod 755 "$BIN_DIR/concorde"

# 9b) Install static library (rename to libconcorde.a)
echo "    • Installing static library → $LIB_DIR/libconcorde.a"
sudo cp "$BUILT_LIB" "$LIB_DIR/libconcorde.a"
sudo chmod 644 "$LIB_DIR/libconcorde.a"

# 9c) Install header files from $HDR_DIR/
echo "    • Installing headers into $INCLUDE_DIR/"
sudo cp -r "$HDR_DIR"/* "$INCLUDE_DIR/"

# If there's a top-level “concorde.h” (generated), install it too
if [ -f concorde.h ]; then
  echo "    • Installing top-level header: concorde.h → $INCLUDE_DIR/"
  sudo cp concorde.h "$INCLUDE_DIR/concorde.h"
  sudo chmod 644 "$INCLUDE_DIR/concorde.h"
fi

echo

# ──────────────────────────────────────────────────────────────────────────────
# 10) (Optional) Update shared-library cache
# ──────────────────────────────────────────────────────────────────────────────
# On macOS, static libs (.a) do not require ldconfig. We skip this step.
echo "==> Done. Concorde CLI, library, and headers installed under: $PREFIX"
echo "    - CLI:            $BIN_DIR/concorde"
echo "    - Static library: $LIB_DIR/libconcorde.a"
echo "    - Headers:        $INCLUDE_DIR/*.h"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 11) (Optional) Clean up source folder
# ──────────────────────────────────────────────────────────────────────────────
# If you wish to remove the Concorde source directory after installation,
# uncomment the following lines:
#
# cd ..
# echo "==> Removing Concorde source directory…"
# rm -rf concorde
#
# ──────────────────────────────────────────────────────────────────────────────

cd ..

exit 0
