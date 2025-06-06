#!/usr/bin/env bash
set -e

# install_visualize.sh
# Installs visualize.py as a system-wide command on Raspberry Pi OS (Debian-based).
# Ensures python3, pip3, and matplotlib are installed before copying the script.

# 1. Require root privileges
if [[ $EUID -ne 0 ]]; then
    echo "Error: This script must be run with sudo or as root."
    exit 1
fi

# 2. Ensure apt cache is up to date
apt update

# 3. Install python3 if missing
if ! command -v python3 >/dev/null 2>&1; then
    apt install -y python3
fi

# 4. Install pip3 if missing
if ! command -v pip3 >/dev/null 2>&1; then
    apt install -y python3-pip
fi

# 5. Install matplotlib via APT if not already present
#    (python3-matplotlib is the Debian package for system-wide install)
if ! python3 -c "import matplotlib" >/dev/null 2>&1; then
    apt install -y python3-matplotlib
fi

# 6. Verify that visualize.py exists
SCRIPT_SRC="app/python/visualize.py"
if [[ ! -f "$SCRIPT_SRC" ]]; then
    echo "Error: $SCRIPT_SRC not found in $(pwd)."
    echo "Please place visualize.py in this directory and re-run."
    exit 1
fi

# 7. Install visualize.py to /usr/local/bin as “visualize”
DEST="/usr/local/bin/visualize"
install -m 0755 "$SCRIPT_SRC" "$DEST"

echo "✓ Installed visualize to $DEST"
echo "✓ Ensure /usr/local/bin is in your PATH to run it as: visualize <coords.csv> <solution.sol> <output.png>"
