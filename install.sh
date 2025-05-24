# Install CMake to build executable
sudo apt install cmake -y

# Install and set up GPS drivers
sudo apt install gpsd gpsd-clients -y
sudo systemctl stop gpsd.socket
sudo systemctl disable gpsd.socket
sudo gpsd /dev/ttyACM0 -F /var/run/gpsd.sock

# Go into build directory
cd app/build/

# Build and install executable
cmake -DCMAKE_BUILD_TYPE=release -DENABLE_COVERAGE=OFF ..
cmake --build .
sudo mv awns-rpi5 /usr/local/bin/
