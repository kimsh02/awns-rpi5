# Script to pull and rebuild executable on Raspberry Pi
git pull
cd app/build/
cmake -DCMAKE_BUILD_TYPE=release -DENABLE_COVERAGE=OFF ..
cmake --build .
sudo mv awns-rpi5 /usr/local/bin/
awns-rpi5
