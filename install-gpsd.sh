# Install CMake
sudo apt install cmake -y

# Install and set up GPS drivers
sudo apt install gpsd gpsd-clients -y
sudo systemctl stop gpsd.socket
sudo systemctl disable gpsd.socket
sudo gpsd /dev/ttyACM0 -F /var/run/gpsd.sock
