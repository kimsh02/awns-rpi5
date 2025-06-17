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

- See the help message below.

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

## File Input/Output

- The `awns-rpi5` program invoked with `run` or `solve` will expect the user to
  provide certain files and/or directories and is explained below.

### CSV File

- The program will read in a series of static waypoints via a CSV file.

-



## API



## Development

- This project is compiled using CMake.

- There is a script `concorde-macos-arm.sh` to install the Concorde/Linkern
  binary on a Silicon chip MacOS computer.