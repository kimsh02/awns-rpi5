#!/usr/bin/env bash
#===============================================================================
# install-concorde.sh
#
# System‐wide installation of the Concorde TSP Solver on:
#   • Raspberry Pi 5 OS Lite (Bookworm, aarch64)
#
# Usage:
#   chmod +x install-concorde.sh
#   ./install-concorde.sh
#
# This script will re‐invoke itself under sudo if not already root.
#===============================================================================

set -euo pipefail

# 1) Re‐exec under sudo if not root:
if [[ "$EUID" -ne 0 ]]; then
  echo "Elevating privileges with sudo..."
  exec sudo bash "$0" "$@"
fi

OS="$(uname -s)"
ARCH="$(uname -m)"

echo "Detected OS:   $OS"
echo "Detected Arch: $ARCH"
echo

#-------------------------------------------------------------------------------
# Function: install_on_pi5
#   - Clones the Concorde repository from the matthelb mirror
#   - Configures + compiles the solver (no HiGHS/LP dependencies)
#-------------------------------------------------------------------------------
install_on_pi5() {
  echo "=== Installing Concorde TSP Solver on Raspberry Pi 5 OS Lite ==="
  echo

  # 2) Update APT and install build dependencies:
  echo "Updating APT repositories..."
  apt-get update  	                                             [oai_citation:0‡Debian Wiki](https://wiki.debian.org/RaspberryPi?utm_source=chatgpt.com)

  echo "Installing build-essential, gfortran, GMP, BLAS/LAPACK, pkg-config, autoconf, automake, libtool, git..."
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
  # • build-essential: gcc/g++/make, etc.
  # • gfortran: Fortran compiler (Concorde’s code has Fortran‐linked components)
  # • libgmp-dev: Arbitrary‐precision arithmetic (used by Concorde)  [oai_citation:1‡Wikipedia](https://en.wikipedia.org/wiki/Concorde_TSP_Solver?utm_source=chatgpt.com)
  # • liblapack-dev, libblas-dev: Linear algebra backends (Concorde uses LAPACK routines)  [oai_citation:2‡Wikipedia](https://en.wikipedia.org/wiki/Concorde_TSP_Solver?utm_source=chatgpt.com)
  # • autoconf/automake/libtool: Used to generate configure scripts  [oai_citation:3‡Wikipedia](https://en.wikipedia.org/wiki/Concorde_TSP_Solver?utm_source=chatgpt.com)
  # • git: To clone the repository  [oai_citation:4‡GitHub](https://github.com/matthelb/concorde?utm_source=chatgpt.com)
  echo

  # 3) Set up source directories:
  SRC_DIR="/usr/local/src"
  CONCORDE_DIR="${SRC_DIR}/concorde"

  echo "Ensuring ${SRC_DIR} exists and is writable..."
  mkdir -p "$SRC_DIR"
  cd "$SRC_DIR"

  # 4) Clone the Concorde repository (matthelb/concorde)  [oai_citation:5‡GitHub](https://github.com/matthelb/concorde?utm_source=chatgpt.com):
  echo "Cloning Concorde repository from https://github.com/matthelb/concorde.git (stable)…"
  rm -rf "$CONCORDE_DIR"
  git clone --depth 1 --branch master https://github.com/matthelb/concorde.git "$CONCORDE_DIR"
  # Note: The “master” branch of matthelb/concorde tracks Concorde’s 03.12.19 release.
  echo

  # 5) Enter the Concorde source subdirectory:
  cd "${CONCORDE_DIR}/CONCORDE"

  # 6) Generate the configure script (autoreconf)  [oai_citation:6‡Wikipedia](https://en.wikipedia.org/wiki/Concorde_TSP_Solver?utm_source=chatgpt.com):
  echo "Running autoreconf to generate configure script…"
  autoreconf -fi
  echo

  # 7) Configure Concorde (no external LP solver). By default, Concorde’s configure script
  #    will disable LP‐solver‐specific options if none is found.  [oai_citation:7‡Wikipedia](https://en.wikipedia.org/wiki/Concorde_TSP_Solver?utm_source=chatgpt.com)
  echo "Configuring Concorde build (auto-detect options)..."
  ./configure
  echo

  # 8) Compile Concorde (this may take ~10–20 minutes on a Pi 5)  [oai_citation:8‡Wikipedia](https://en.wikipedia.org/wiki/Concorde_TSP_Solver?utm_source=chatgpt.com):
  echo "Building Concorde (this may take ~10–20 minutes)..."
  make
  echo

  # 9) Install Concorde system‐wide:
  echo "Installing Concorde to /usr/local/bin…"
  make install
  echo

  echo "✅ Concorde TSP Solver installed under /usr/local/bin."
  echo "   Verify by running:  concorde -v"
  echo
}

#-------------------------------------------------------------------------------
# Main dispatcher
#-------------------------------------------------------------------------------
case "$OS" in
  Linux)
    # Only proceed if this is a Raspberry Pi 5 (Bookworm, aarch64)
    if [[ "$ARCH" == "aarch64" && -f /proc/device-tree/model ]]; then
      MODEL="$(tr -d '\0' < /proc/device-tree/model)"
      if [[ "$MODEL" == *"Raspberry Pi 5"* ]]; then
        install_on_pi5
      else
        echo "Unsupported device model: $MODEL"
        echo "This installer only supports Raspberry Pi 5."
        exit 1
      fi
    else
      echo "Unsupported Linux architecture or missing device-tree: $ARCH"
      echo "This installer only supports Raspberry Pi 5 OS Lite (aarch64)."
      exit 1
    fi
    ;;

  *)
    echo "Unsupported platform: $OS"
    exit 1
    ;;
esac

echo "✅ Installation script completed successfully."
