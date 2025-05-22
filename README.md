# Autonomous Waypoint Navigation System for Raspberry Pi 5 (awns-rpi5)


## About

- This software project enables autonomous waypoint navigation for a mobile
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

1. Flash Raspberry Pi OS Lite (64-bit) onto your Raspberry Pi 5. Make sure to
enable ssh and wifi in the Raspberry Pi imager.

2. Ssh into your Raspberry Pi and update the OS using `sudo apt update && sudo
apt upgrade -y`.

3. Install git using `sudo apt install git` so that we can clone this software
project through git.

4. Install the GPS driver using `sudo apt install gpsd`.

5. Clone this project using `git clone
https://github.com/kimsh02/awns-rpi5.git`.

6. Go into the project directory with `cd awns-rpi5`.

7. Run `./install-docker.sh`. This will install docker as this is a docker
project.
