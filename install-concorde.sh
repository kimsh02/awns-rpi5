#!/usr/bin/env bash
#
# install-concorde.sh
#
# This script clones Concorde (if needed), applies two small patches
# to avoid conflicts (u_char / gethostname), builds only the Concorde
# CLI executable, and installs it into /usr/local/bin (or a user‐supplied
# prefix).  It does NOT install Concorde’s libraries or headers—only
# the concorde binary itself.
#
# Usage:
#   chmod +x install-concorde.sh
#   ./install-concorde.sh [<install_prefix>]
#
# If you omit <install_prefix>, it defaults to /usr/local.
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

echo "==> Installing recommended libraries (if not already present)..."
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
# 3) Apply small patches (avoid u_char / gethostname conflicts)
# ----------------------------------------------------------------------------–
echo "==> Applying in‐place patches to avoid u_char / gethostname conflicts..."

# Ensure we’re inside the Concorde source directory
cd concorde

# 3a) In INCLUDE/config.h, comment out "#define u_char unsigned char"
if grep -q '^#define u_char unsigned char' INCLUDE/config.h; then
  sed -i.bak 's|^#define u_char unsigned char|// #define u_char unsigned char|' INCLUDE/config.h
  echo "    • Commented out u_char in INCLUDE/config.h"
else
  echo "    • (Already patched) u_char define not found in INCLUDE/config.h"
fi

# 3b) In INCLUDE/machdefs.h, comment out any "gethostname(char *, int);" prototype
#      (the system-provided gethostname in unistd.h expects size_t length).
if grep -q '^[[:space:]]*gethostname *(char \*, *int);' INCLUDE/machdefs.h; then
  sed -i.bak -E 's|^[[:space:]]*(gethostname *\( *char \*, *int *\);)|// \1|' INCLUDE/machdefs.h
  echo "    • Commented out gethostname prototype in INCLUDE/machdefs.h"
else
  echo "    • (Already patched) gethostname prototype not found in INCLUDE/machdefs.h"
fi

echo

# ----------------------------------------------------------------------------–
# 4) Prepare build scripts
# ----------------------------------------------------------------------------–
echo "==> Making configure, config.guess, config.sub executable..."
chmod +x configure
find . -type f \( -name "config.guess" -o -name "config.sub" \) -exec chmod +x {} \;

echo

# ----------------------------------------------------------------------------–
# 5) Configure Concorde (top‐level)
# ----------------------------------------------------------------------------–
echo "==> Running ./configure --prefix=$PREFIX"
./configure --prefix="$PREFIX"
echo

# ----------------------------------------------------------------------------–
# 6) Build Concorde (full 'make')—we only need to find the CLI afterward
# ----------------------------------------------------------------------------–
echo "==> Running make -j to compile Concorde"
CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
make -j"$CORES"
echo

# ----------------------------------------------------------------------------–
# 7) Locate the built 'concorde' binary
# ----------------------------------------------------------------------------–
echo "==> Locating the 'concorde' CLI binary after build..."

BUILT_BIN=""
if [ -x utilities/concorde/concorde ]; then
  BUILT_BIN="utilities/concorde/concorde"
elif [ -x utilities/concorde/concorde.exe ]; then
  BUILT_BIN="utilities/concorde/concorde.exe"
elif [ -x TSP/concorde ]; then
  BUILT_BIN="TSP/concorde"
elif [ -x ./concorde ]; then
  BUILT_BIN="./concorde"
else
  # Fallback: search for any executable named "concorde" under subdirectories
  FOUND=$(find . -type f -name concorde -perm /u+x 2>/dev/null | head -n 1 || echo "")
  if [ -n "$FOUND" ]; then
    BUILT_BIN="$FOUND"
  fi
fi

if [ -z "$BUILT_BIN" ]; then
  echo "Error: Cannot locate the built 'concorde' executable."
  exit 1
fi

echo "    • Found Concorde binary at: $BUILT_BIN"
echo

# ----------------------------------------------------------------------------–
# 8) Install the 'concorde' binary into $PREFIX/bin
# ----------------------------------------------------------------------------–
echo "==> Installing 'concorde' to $BIN_DIR"
mkdir -p "$BIN_DIR"
sudo cp "$BUILT_BIN" "$BIN_DIR/concorde"
sudo chmod 755 "$BIN_DIR/concorde"

echo
echo "✔ Concorde CLI installed to: $BIN_DIR/concorde"
echo "  You can verify with: concorde -h"
echo

# ----------------------------------------------------------------------------–
# 9) (Optional) Clean up Concorde source folder
# ----------------------------------------------------------------------------–
# If you prefer to remove the source after installing the binary, uncomment:
#
# cd ..
# rm -rf concorde
#
# ----------------------------------------------------------------------------–

cd ..
