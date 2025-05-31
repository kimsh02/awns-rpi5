#!/usr/bin/env bash
#===============================================================================
# install-googleortools.sh
#
# System‐wide installation of Google OR-Tools on:
#   • macOS (Apple Silicon/M2) via Homebrew
#   • Raspberry Pi 5 OS Lite (aarch64) by building from source (HiGHS disabled)
#
# Usage:
#   chmod +x install-googleortools.sh
#   ./install-googleortools.sh
#
# This script re‐invokes itself under sudo if not already root.
#===============================================================================

set -euo pipefail

# Re‐exec under sudo if not root
if [[ "$EUID" -ne 0 ]]; then
  echo "Elevating privileges with sudo..."
  exec sudo bash "$0" "$@"
fi

OS="$(uname -s)"
ARCH="$(uname -m)"

echo "Detected OS:   $OS"
echo "Detected Arch: $ARCH"
echo

install_on_macos() {
  echo "=== Installing OR-Tools on macOS (Apple Silicon/M2) ==="
  echo

  if ! command -v brew &>/dev/null; then
    echo "Homebrew not found. Install Homebrew first: https://brew.sh/"
    exit 1
  fi

  echo "Updating Homebrew..."
  brew update

  echo "Installing OR-Tools via Homebrew..."
  brew install or-tools

  echo
  echo "✅ OR-Tools installed via Homebrew."
  echo "   Verify with: or-tools-config --version"
  echo
}

install_on_pi5() {
  echo "=== Installing OR-Tools on Raspberry Pi 5 OS Lite ==="
  echo

  echo "Updating APT repositories..."
  apt-get update

  echo "Installing build dependencies..."
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
    libabsl-dev       # Abseil C++ libraries
    libre2-dev        # RE2 regex library
    # HiGHS unavailable on Pi OS Bookworm → disable below instead of installing libhighs-dev
  )
  apt-get install -y "${DEPS[@]}"

  SRC_DIR="/usr/local/src"
  BUILD_DIR="/usr/local/src/ortools-build"
  ORTOOLS_SRC_DIR="${SRC_DIR}/or-tools"

  echo "Ensuring source directory exists and is writable..."
  mkdir -p "$SRC_DIR"
  cd "$SRC_DIR"

  echo "Cloning OR-Tools (stable branch)…"
  rm -rf "$ORTOOLS_SRC_DIR"
  git clone --depth 1 --branch stable https://github.com/google/or-tools.git "$ORTOOLS_SRC_DIR"

  echo "Creating build directory…"
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"

  echo "Configuring with CMake (disable HiGHS)…"
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DBUILD_HIGHS=OFF \
    "$ORTOOLS_SRC_DIR"

  echo "Building OR-Tools (this may take ~20–30 minutes)…"
  make -j"$(nproc)"

  echo "Installing OR-Tools system-wide…"
  make install

  echo
  echo "✅ OR-Tools built and installed under /usr/local."
  echo "   Verify with: or-tools-config --version"
  echo
}

case "$OS" in
  Darwin)
    if [[ "$ARCH" == "arm64" ]]; then
      install_on_macos
    else
      echo "Unsupported macOS architecture: $ARCH"
      exit 1
    fi
    ;;

  Linux)
    # Check for Raspberry Pi 5
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
