# Install CMake to build executable
sudo apt install cmake -y

# Install and set up GPS drivers/libraries
sudo apt install gpsd gpsd-clients libgps-dev -y
sudo systemctl stop gpsd.socket
sudo gpsd -G /dev/ttyACM0 -F /var/run/gpsd.sock

# Install concorde
./scripts/concorde-debian.sh

# Go into build directory
cd app/build/

# Build and install executable
cmake -DCMAKE_BUILD_TYPE=release -DENABLE_COVERAGE=OFF ..
cmake --build .
sudo mv awns-rpi5 /usr/local/bin/
