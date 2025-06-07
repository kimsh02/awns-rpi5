#!/usr/bin/env bash
#
# install-concorde-and-linkern.sh
#
# This script clones (or updates) the Concorde repository, applies small patches
# to avoid u_char / gethostname conflicts, builds Concorde’s static library,
# the "concorde" CLI, and the "linkern" heuristic binary, and installs them into:
#   <PREFIX>/bin/concorde
#   <PREFIX>/bin/linkern
#   <PREFIX>/lib/libconcorde.a
#   <PREFIX>/include/  (all Concorde header files)
#
# Usage:
#   chmod +x install-concorde-and-linkern.sh
#   sudo ./install-concorde-and-linkern.sh [<install_prefix>]
#
# If you omit <install_prefix>, it defaults to /usr/local.
#
set -euo pipefail

PREFIX="${1:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib"
INCLUDE_DIR="$PREFIX/include"

echo "Installing Concorde, CLI, headers, and LINKERN into prefix: $PREFIX"
echo

# ──────────────────────────────────────────────────────────────────────────────
# 1) Ensure build prerequisites are installed
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Verifying build tools (gcc, make, git)…"
command -v gcc >/dev/null 2>&1 || { echo "Error: gcc not found. Install 'build-essential'." >&2; exit 1; }
command -v make >/dev/null 2>&1 || { echo "Error: make not found. Install 'build-essential'." >&2; exit 1; }
command -v git >/dev/null 2>&1 || { echo "Error: git not found. Install 'git'." >&2; exit 1; }

echo "==> Installing recommended libraries…"
sudo apt-get update
sudo apt-get install -y libgmp-dev liblapack-dev libblas-dev \
                          libreadline-dev libbz2-dev flex bison pkg-config build-essential

echo
# ──────────────────────────────────────────────────────────────────────────────
# 2) Clone or update the Concorde repository
# ──────────────────────────────────────────────────────────────────────────────
if [ ! -d concorde ]; then
  echo "==> Cloning Concorde repository…"
  git clone https://github.com/matthelb/concorde.git
else
  echo "==> 'concorde' directory exists. Pulling latest…"
  cd concorde && git fetch origin && git checkout master && git pull && cd ..
fi

echo
# ──────────────────────────────────────────────────────────────────────────────
# 3) Apply patches to avoid u_char/gethostname conflicts
# ──────────────────────────────────────────────────────────────────────────────
cd concorde

echo "==> Patching headers…"
# config.h
if grep -q '^#define u_char unsigned char' INCLUDE/config.h; then
  sed -i.bak 's|^#define u_char unsigned char|// #define u_char unsigned char|' INCLUDE/config.h
  echo "    • Commented out u_char define"
fi
# machdefs.h
if grep -q '^[[:space:]]*gethostname *(char \, *int);' INCLUDE/machdefs.h; then
  sed -i.bak -E 's|^[[:space:]]*(gethostname *\( *char \, *int *\);)|// \1|' INCLUDE/machdefs.h
  echo "    • Commented out gethostname prototype"
fi

echo
# ──────────────────────────────────────────────────────────────────────────────
# 4) Make configure, config.guess, config.sub executable
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Making build scripts executable…"
chmod +x configure
find . -type f \( -name "config.guess" -o -name "config.sub" \) -exec chmod +x {} +

echo
# ──────────────────────────────────────────────────────────────────────────────
# 5) Configure Concorde (top-level)
# ──────────────────────────────────────────────────────────────────────────────
echo "==> ./configure --prefix=$PREFIX"
./configure --prefix="$PREFIX"
echo
# ──────────────────────────────────────────────────────────────────────────────
# 6) Build Concorde and LINKERN
# ──────────────────────────────────────────────────────────────────────────────
CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
echo "==> make -j$CORES (Concorde CLI & static lib)"
make -j"$CORES"
echo "==> make -C LINKERN (build Lin–Kernighan heuristic)"
make -C LINKERN

echo
# ──────────────────────────────────────────────────────────────────────────────
# 7) Install CLI, library, headers, and LINKERN
# ──────────────────────────────────────────────────────────────────────────────
echo "==> Creating directories: $BIN_DIR, $LIB_DIR, $INCLUDE_DIR"
sudo mkdir -p "$BIN_DIR" "$LIB_DIR" "$INCLUDE_DIR"

echo "==> Installing Concorde CLI → $BIN_DIR/concorde"
sudo cp utilities/concorde/concorde "$BIN_DIR/concorde"
sudo chmod 755 "$BIN_DIR/concorde"

echo "==> Installing static library → $LIB_DIR/libconcorde.a"
sudo cp concorde.a "$LIB_DIR/libconcorde.a"
sudo chmod 644 "$LIB_DIR/libconcorde.a"

echo "==> Installing headers → $INCLUDE_DIR/"
sudo cp -r INCLUDE/* "$INCLUDE_DIR/"
[ -f concorde.h ] && sudo cp concorde.h "$INCLUDE_DIR/" && sudo chmod 644 "$INCLUDE_DIR/concorde.h"

echo "==> Installing LINKERN CLI → $BIN_DIR/linkern"
sudo cp LINKERN/linkern "$BIN_DIR/linkern"
sudo chmod 755 "$BIN_DIR/linkern"

echo
# ──────────────────────────────────────────────────────────────────────────────
# 8) Update linker cache (optional)
# ──────────────────────────────────────────────────────────────────────────────
if command -v ldconfig >/dev/null 2>&1; then
  echo "==> ldconfig"
  sudo ldconfig
fi

echo "✔ Installed Concorde & LINKERN under $PREFIX"
cd ..
