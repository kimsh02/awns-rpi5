#!/usr/bin/env bash
#===============================================================================
# install_ortools.sh
#
# System‐wide installation of Google OR-Tools on:
#   • macOS (Apple Silicon/M2) using Homebrew
#   • Raspberry Pi OS Lite (ARM) by building from source
#
# Usage:
#   chmod +x install_ortools.sh
#   sudo ./install_ortools.sh
#
# This script detects the platform and installs OR-Tools accordingly.
# It must be run with root privileges (or via sudo) for system-wide installs.
#===============================================================================

set -euo pipefail

# Detect OS and architecture
OS="$(uname -s)"
ARCH="$(uname -m)"

echo "Detected OS: $OS"
echo "Detected Architecture: $ARCH"
echo

#-----------------------------------------------------------------------------
# macOS (Darwin) on Apple Silicon (arm64)
#-----------------------------------------------------------------------------

install_on_macos() {
  echo "=== Installing OR-Tools on macOS (Apple Silicon/M2) ==="
  echo

  # Check for Homebrew
  if ! command -v brew &>/dev/null; then
    echo "Homebrew not found. Please install Homebrew first:"
    echo "  https://brew.sh/"
    exit 1
  fi

  echo "Updating Homebrew..."
  brew update

  echo "Installing OR-Tools via Homebrew..."
  # The formula name is “or-tools”
  brew install or-tools

  echo
  echo "OR-Tools has been installed system-wide via Homebrew."
  echo "You can verify by running:  or-tools-config --version"
  echo
}

#-----------------------------------------------------------------------------
# Raspberry Pi OS Lite (Debian‐based ARM)
#-----------------------------------------------------------------------------

install_on_rpi() {
  echo "=== Installing OR-Tools on Raspberry Pi OS Lite (ARM) ==="
  echo

  echo "Updating APT repositories..."
  apt-get update

  echo "Installing build-dependencies..."
  DEPS=(
    build-essential
    cmake
    git
    python3-dev
    libgflags-dev
    libgoogle-glog-dev
    libprotobuf-dev
    protobuf-compiler
    libeigen3-dev
    pkg-config
    libatlas-base-dev
  )
  apt-get install -y "${DEPS[@]}"

  # Choose OR-Tools version (modify if you want a different tag)
  ORTOOLS_TAG="v9.9.10497"
  SRC_DIR="/usr/local/src"
  BUILD_DIR="/usr/local/src/ortools-build"

  echo "Cloning OR-Tools (tag: $ORTOOLS_TAG)..."
  cd "$SRC_DIR"
  if [ -d "ortools" ]; then
    echo "Directory '$SRC_DIR/ortools' already exists—removing it first."
    rm -rf "ortools"
  fi
  git clone --depth 1 --branch "$ORTOOLS_TAG" https://github.com/google/or-tools.git

  echo "Creating build directory..."
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"

  echo "Configuring with CMake..."
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    "$SRC_DIR/ortools"

  echo "Building OR-Tools (this may take ~20-30 minutes)..."
  make -j"$(nproc)"

  echo "Installing OR-Tools system-wide..."
  make install

  echo
  echo "OR-Tools has been built and installed under /usr/local."
  echo "Verify by running:  or-tools-config --version"
  echo
}

#-----------------------------------------------------------------------------
# Main dispatcher
#-----------------------------------------------------------------------------

case "$OS" in
  Darwin)
    if [ "$ARCH" = "arm64" ]; then
      install_on_macos
    else
      echo "Unsupported architecture on macOS: $ARCH"
      echo "This script only supports Apple Silicon (arm64)."
      exit 1
    fi
    ;;

  Linux)
    # Assume Raspberry Pi OS Lite if running on ARM
    if [[ "$ARCH" == arm* ]]; then
      install_on_rpi
    else
      echo "Unsupported architecture on Linux: $ARCH"
      echo "This script is intended for Raspberry Pi OS Lite (ARM)."
      exit 1
    fi
    ;;

  *)
    echo "Unsupported platform: $OS"
    exit 1
    ;;
esac

echo "Installation script has finished successfully."
