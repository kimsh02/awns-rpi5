#!/usr/bin/env bash
#
# install-concorde.sh
#
# This script clones (or updates) the Concorde repository, applies small patches
# to avoid u_char / gethostname conflicts, builds Concorde’s static library
# and the “concorde” CLI, and installs them into:
#   <PREFIX>/bin/concorde
#   <PREFIX>/lib/libconcorde.a
#   <PREFIX>/include/  (all Concorde header files)
#
# Usage:
#   chmod +x install-concorde.sh
#   sudo ./install-concorde.sh [<install_prefix>]
#
# If you omit <install_prefix>, it defaults to /usr/local.
#

set -euo pipefail

PREFIX="${1:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib"
INCLUDE_DIR="$PREFIX/include"

echo "Installing Concorde (CLI, library, headers) into prefix: $PREFIX"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 1) Ensure build prerequisites are installed
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Verifying build tools (gcc, make, git)…"
command -v gcc >/dev/null 2>&1 || {
  echo "Error: gcc not found. Please install 'build-essential' first."
  exit 1
}
command -v make >/dev/null 2>&1 || {
  echo "Error: make not found. Please install 'build-essential' first."
  exit 1
}
command -v git >/dev/null 2>&1 || {
  echo "Error: git not found. Please install git."
  exit 1
}

echo "==> Installing recommended libraries (if not already present)…"
apt-get update
apt-get install -y \
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

# ──────────────────────────────────────────────────────────────────────────────
# 2) Clone or update the Concorde repository
# ──────────────────────────────────────────────────────────────────────────────
if [ ! -d concorde ]; then
  echo "==> Cloning Concorde repository…"
  git clone https://github.com/matthelb/concorde.git
else
  echo "==> 'concorde' directory already exists. Pulling latest changes…"
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

# 3a) In INCLUDE/config.h, comment out "#define u_char unsigned char"
if grep -q '^#define u_char unsigned char' INCLUDE/config.h; then
  sed -i.bak \
    's|^#define u_char unsigned char|// #define u_char unsigned char|' \
    INCLUDE/config.h
  echo "    • Commented out u_char in INCLUDE/config.h"
else
  echo "    • (Already patched) u_char define not found in INCLUDE/config.h"
fi

# 3b) In INCLUDE/machdefs.h, comment out "gethostname(char *, int);"
if grep -q '^[[:space:]]*gethostname *(char \*, *int);' INCLUDE/machdefs.h; then
  sed -i.bak -E \
    's|^[[:space:]]*(gethostname *\( *char \*, *int *\);)|// \1|' \
    INCLUDE/machdefs.h
  echo "    • Commented out gethostname prototype in INCLUDE/machdefs.h"
else
  echo "    • (Already patched) gethostname prototype not found in INCLUDE/machdefs.h"
fi

echo

# ──────────────────────────────────────────────────────────────────────────────
# 4) Make configure, config.guess, config.sub executable
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Making configure, config.guess, config.sub executable…"
chmod +x configure
find . -type f \( -name "config.guess" -o -name "config.sub" \) \
     -exec chmod +x {} \;

echo

# ──────────────────────────────────────────────────────────────────────────────
# 5) Configure Concorde (top‐level)
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Running ./configure --prefix=$PREFIX"
./configure --prefix="$PREFIX"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 6) Build Concorde (static library + CLI)
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Running make -j to compile Concorde"
CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
make -j"$CORES"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 7) Locate the built 'concorde' CLI binary
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Locating the 'concorde' CLI binary…"
BUILT_CLI=""
if [ -x utilities/concorde/concorde ]; then
  BUILT_CLI="utilities/concorde/concorde"
elif [ -x utilities/concorde/concorde.exe ]; then
  BUILT_CLI="utilities/concorde/concorde.exe"
elif [ -x TSP/concorde ]; then
  BUILT_CLI="TSP/concorde"
elif [ -x ./concorde ]; then
  BUILT_CLI="./concorde"
else
  # Fallback: search for any executable named "concorde"
  FOUND=$(find . -type f -name concorde -perm /u+x 2>/dev/null | head -n 1 || echo "")
  if [ -n "$FOUND" ]; then
    BUILT_CLI="$FOUND"
  fi
fi

if [ -z "$BUILT_CLI" ]; then
  echo "Error: Cannot locate the built 'concorde' executable."
  exit 1
fi

echo "    • Found Concorde CLI at: $BUILT_CLI"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 8) Locate the built static library (concorde.a)
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Locating the Concorde static library…"
if [ -f concorde.a ]; then
  BUILT_LIB="concorde.a"
elif [ -f libconcorde.a ]; then
  BUILT_LIB="libconcorde.a"
else
  # Fallback: look for "concorde.a" under subdirectories
  FOUNDLIB=$(find . -type f -name "concorde.a" 2>/dev/null | head -n 1 || echo "")
  if [ -n "$FOUNDLIB" ]; then
    BUILT_LIB="$FOUNDLIB"
  else
    echo "Error: Cannot locate the Concorde static library (concorde.a)."
    exit 1
  fi
fi

echo "    • Found Concorde static library at: $BUILT_LIB"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 9) Install CLI, library, and headers
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Installing Concorde components…"
echo "    • Creating directories:"
echo "         bin → $BIN_DIR"
echo "         lib → $LIB_DIR"
echo "       include → $INCLUDE_DIR"
mkdir -p "$BIN_DIR" "$LIB_DIR" "$INCLUDE_DIR"

# 9a) Install the CLI:
echo "    • Installing CLI: $BUILT_CLI → $BIN_DIR/concorde"
cp "$BUILT_CLI" "$BIN_DIR/concorde"
chmod 755 "$BIN_DIR/concorde"

# 9b) Install the static library:
#     Rename to libconcorde.a if needed.
echo "    • Installing static library: $BUILT_LIB → $LIB_DIR/libconcorde.a"
cp "$BUILT_LIB" "$LIB_DIR/libconcorde.a"
chmod 644 "$LIB_DIR/libconcorde.a"

# 9c) Install header files (everything under INCLUDE/)
#     We’ll copy all *.h files into $INCLUDE_DIR so you can do #include <config.h> etc.
echo "    • Installing header files into $INCLUDE_DIR/"
cp -r INCLUDE/* "$INCLUDE_DIR/"
# Also install any top‐level generated header (e.g. concorde.h if present)
if [ -f concorde.h ]; then
  echo "    • Installing top‐level header: concorde.h → $INCLUDE_DIR/"
  cp concorde.h "$INCLUDE_DIR/"
  chmod 644 "$INCLUDE_DIR/concorde.h"
fi

echo

# ──────────────────────────────────────────────────────────────────────────────
# 10) Update shared‐library cache (for completeness, though we installed a static .a)
# ──────────────────────────────────────────────────────────────────────────────
if command -v ldconfig >/dev/null 2>&1; then
  echo "==> Running ldconfig to update linker cache"
  ldconfig
  echo
fi

echo "✔ Concorde CLI, library, and headers installed under: $PREFIX"
echo "  - CLI:    $BIN_DIR/concorde"
echo "  - Static library: $LIB_DIR/libconcorde.a"
echo "  - Headers: $INCLUDE_DIR/{*.h,...}"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 11) (Optional) Clean up Concorde source folder
# ──────────────────────────────────────────────────────────────────────────────
# If you prefer to remove the entire source directory after installation,
# uncomment the following lines:
#
# cd ..
# echo "==> Removing Concorde source directory…"
# rm -rf concorde
#
# ──────────────────────────────────────────────────────────────────────────────

cd ..

echo "All done!"
