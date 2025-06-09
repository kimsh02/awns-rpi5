# Install CMake to build executable
apt install cmake git -y

# Install and set up GPS drivers/libraries
apt install gpsd gpsd-clients libgps-dev nlohmann-json3-dev -y
systemctl stop gpsd.socket
gpsd -G /dev/ttyACM0 -F /var/run/gpsd.sock

# Install concorde
./scripts/concorde-debian.sh

# Install python libraries
./scripts/python-plot.sh

# Go into build directory
cd app/build/

# Build and install executable
cmake -DCMAKE_BUILD_TYPE=release -DENABLE_COVERAGE=OFF ..
cmake --build .
cp awns-rpi5 /usr/local/bin/
