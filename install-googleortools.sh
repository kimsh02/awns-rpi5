#!/usr/bin/env bash
#===============================================================================
# install-concorde.sh
#
# System‐wide installation of the Concorde TSP solver on:
#   • Raspberry Pi 5 OS Lite (aarch64)
#
# Usage:
#   chmod +x install-concorde.sh
#   ./install-concorde.sh
#
# This script will re‐invoke itself under sudo if not already root.
#===============================================================================

set -euo pipefail

# Re‐exec under sudo if we’re not root
if [[ "$EUID" -ne 0 ]]; then
  echo "Elevating privileges with sudo..."
  exec sudo bash "$0" "$@"
fi

OS="$(uname -s)"
ARCH="$(uname -m)"

echo "Detected OS:   $OS"
echo "Detected Arch: $ARCH"
echo

install_on_pi5() {
  echo "=== Installing Concorde TSP Solver on Raspberry Pi 5 OS Lite ==="
  echo

  # 1) Update and install build dependencies
  echo "Updating APT repositories..."
  apt-get update

  echo "Installing build-essential, gfortran, GMP, BLAS/LAPACK, pkg-config..."
  apt-get install -y \
    build-essential \
    gfortran \
    git \
    libgmp-dev \
    liblapack-dev \
    libblas-dev \
    pkg-config \
    autoconf \
    automake \
    libtool

  # 2) Clone the Concorde repository
  SRC_DIR="/usr/local/src"
  CONCORDE_DIR="${SRC_DIR}/Concorde"
  BUILD_SUBDIR="${CONCORDE_DIR}/CONCORDE"

  echo "Ensuring source directory exists..."
  mkdir -p "$SRC_DIR"
  cd "$SRC_DIR"

  echo "Cloning Concorde repository (if already exists, it will be removed)..."
  rm -rf "$CONCORDE_DIR"
  git clone https://github.com/jvkersch/Concorde.git "$CONCORDE_DIR"

  # 3) Build Concorde
  echo "Entering Concorde source directory..."
  cd "$BUILD_SUBDIR"

  echo "Generating configure script (autoreconf)..."
  autoreconf -fi

  echo "Configuring Concorde build..."
  ./configure

  echo "Compiling Concorde (this may take ~10–20 minutes)..."
  make

  # 4) Install the Concorde binary and libraries
  echo "Installing Concorde system-wide..."
  make install

  echo
  echo "✅ Concorde TSP Solver installed in /usr/local/bin."
  echo "   Verify by running: concorde --help"
  echo
}

case "$OS" in
  Linux)
    # Verify Raspberry Pi 5 via /proc/device-tree/model
    if [[ "$ARCH" == "aarch64" && -f /proc/device-tree/model ]]; then
      MODEL="$(tr -d '\0' < /proc/device-tree/model)"
      if [[ "$MODEL" == *"Raspberry Pi 5"* ]]; then
        install_on_pi5
      else
        echo "Unsupported device model: $MODEL (only Raspberry Pi 5 supported)"
        exit 1
      fi
    else
      echo "Unsupported Linux architecture or missing device-tree: $ARCH"
      exit 1
    fi
    ;;
  *)
    echo "Unsupported platform: $OS"
    exit 1
    ;;
esac

echo "✅ Installation script completed successfully."
