# Autonomous Waypoint Navigation System for Raspberry Pi 5 (awns-rpi5)

## About

- This software program enables autonomous waypoint navigation for a mobile
platform using a Raspberry Pi 5.

- The system will use GPS data to guide the platform from one location to
another along a predefined series of static waypoints.

## Hardware Requirements

- Raspberry Pi 5 (16GB)

- Raspberry Pi 5 charger (CanaKit 45W USB-C Power Supply with PD for Raspberry
  Pi 5 (27W @ 5A))

- GPS Dongle (VK-162 G-Mouse USB GPS Dongle Navigation Module External GPS
  Antenna Remote Mount USB GPS Receiver for Raspberry Pi Support Google Earth
  Window Linux Geekstory)

- A flash drive (8GB+)

## Installation

1. Plug charger and GPS dongle into your Raspberry Pi.

1. Flash Raspberry Pi OS Lite (64-bit) onto your Raspberry Pi 5
(https://www.raspberrypi.com/software/). Make sure to enable ssh and wifi in the
Raspberry Pi Imager.

1. Ssh into your Raspberry Pi and update the OS using `sudo apt update && sudo
apt upgrade -y`.

1. Install git using `sudo apt install git -y` so that we can clone this
software project through git.

1. Clone this project using `git clone
https://github.com/kimsh02/awns-rpi5.git`.

1. Go into the project directory with `cd awns-rpi5`.

1. Run `sudo ./scripts/install.sh`. This will install, build, and set up the
necessary tools, and then build and install the executable system-wide.

1. Copy paste this one-liner if you want to run the shell commands above in one
   go:
   ```
   sudo apt update && sudo apt upgrade -y && sudo apt install git -y &&
   git clone https://github.com/kimsh02/awns-rpi5.git && cd awns-rpi5 && sudo
   ./scripts/install.sh
   ```

## Usage

- You should now be able to invoke the program with the command `awns-rpi5 COMMAND`.

- Help message print:

```
Usage: awns-rpi5 COMMAND

Autonomous waypoint navigation system for a mobile platform using Raspberry Pi 5

Commands:
  gpspoll        Poll GPS to get a reading
  run            Use GPS data to guide platform along a predefined series of static waypoints and output logs
  solve          Use Concorde TSP to solve directory of CSV waypoint files and output solutions as plotted graphs
  help           Show this help message and exit

Examples:
  awns-rpi5 run
  awns-rpi5 solve
```

## Concorde TSP Solver

- Concorde is a standalone executable written in C and is considered to be the
  world's fastest traveling salesman problem (TSP) solver to date.

- The `install.sh` builds and installs Concorde to the system, and the program
  relies on invoking `concorde` to generate the optimal visiting order of
  waypoints.

- As a sidenote, technically, the system invokes an alternate build of Concorde
  called `linkern` which uses the Lin-Kernighan heuristic in place of the custom
  heuristic used by the full `concorde` binary. The Lin-Kernighan heuristic
  provides near-optimal solutions to TSP but does not offer provably optimal
  solutions which are guaranteed only by the `concorde` executable. The full
  build of `concorde` was not achievable for this project as a specific required
  library `qsopt` was determined to be too out-of-date to be refactored in a
  reasonable amount of time to be in compliance with the C++23 standard (which
  is a build requirement for this project).

## File Input/Output

- The `awns-rpi5` program invoked with `run` or `solve` will expect the user to
  provide certain files and/or directories and is explained below.

### CSV Input

- The program will read in a series of static waypoints via a `.csv` file.

- Must have two columns with headers specifying latitude and longitude and in
  that order.

- A `tests/csv` directory is included in this repo providing test CSV files.

### TSP Output

- The program will generate a `.tsp` file for each `.csv` file and requires a
  directory to output these files.

- The `.tsp` file contains specially converted GPS coordinates and specifies the
  traveling salesman problem in a format that Concorde understands.

- The `.tsp` file is used to communicate to the Concorde TSP solver which reads
  in the `.tsp` file to solve the optimal/near-optimal tour order of waypoints.

- A `tests/tsp` directory is included in this repo for convenient use.

### SOL Output

- The Concorde TSP solver generates a `.sol` file that specifies the solved tour
  order of waypoints and requires a directory to output these files.

- The program will read in the `.sol` file to get the solved tour order.

- A `tests/sol` directory is included in this repo for convenient use.

### Graph Output

- The program invokes a Python script (installed through `install.sh`) to
  generate a `.png` plot visualization of the tour order solved by Concorde and
  requires a directory to output these files.

- A `tests/graph` directory is included in this repo for convenient use.

### LOG Output

- The program in `run` mode optionally provides writing navigation output to a
  `.log` file and requires a directory to output these files.

- A `tests/log` directory is included in this repo for convenient use.

## API

- Simple API that can be integrated with development of a downstream motor
  controller.

### Headers

- `#include "navigator.hpp"`

### Documentation

- `Navigator(int argc, const char **argv) noexcept`
  - @brief Constructor for 'Navigator` object.
  - @param argc The argc from `int main(int argc, const char **argv)`.
  - @param argv The argv from `int main(int argc, const char **argv)`.
  - @return Navigator object.

- `void start(void)`
  - @brief Invoke CLI to setup Navigator object. Must be called after object
    initialization.

- `void setProximityRadius(double r) noexcept`
  - @brief Setter for proximity radius for determining arrival at each waypoint.
  - @param r Proximity radius in meters. Cannot be set to less than 1.0
    meters (will result in default of 1.0).

- `void setSimulationVelocity(double v) noexcept`
  - @brief Optional setter for simulated downstream motor controller speed. If
    set, navigation output is generated based on simulated speed, else
    navigation output is generated based on GPS position.
  - @param v Simulated velocity in meters per second.

- `std::optional<json> getOutput(void)`
  - @brief Spits out navigation output in JSON format and prints output to
    `stdout` and optionally to a `.log` file. Must invoke `start(void)` and
    `setProximityRadius(double r)` beforehand.
  - @return `nlohmann::json` object. JSON key-value of interest for downstream
    controller is 'bearing' whose value is reported as degrees from true North.

- Please see `main.cpp` for example usage of the API.

## Development Notes

- This project is compiled using CMake and its build configuration should be
  managed through `CMakeLists.txt`.

  - There is a `memory`, `thread`, and `release` version of the build and should
    be specified through CMake. While `thread` should not be of interest as this
    program is single-threaded, `memory` offers compilation with a
    memory/address sanitizer (through Clang) for debugging, and `release` offers
    a compilation of the software with `O3` optimization enabled and all other
    debugging flags disabled.

- Script `concorde-macos-arm.sh` installs Concorde/Linkern binary on an
  ARM-based MacOS machine for local development if you have a Mac.

- Script `dev.sh` is a convenience script that re-pulls from Git and re-installs
  executable on Raspberry Pi 5.

- `.gitlab-ci.yml` configures a CI/CD pipeline on GitLab that replicates the
  exact build of `awns-rpi5` on a Raspberry PI 5 OS to ensure that the project
  is building properly without manually having to rebuild on the Raspberry Pi 5
  every time (which is time consuming).

## Artifacts

- [Test Report](artifacts/Autonomous Waypoint Navigation System Test Report.docx)

- [System Presentation](artifacts/Presentation1.pptx)