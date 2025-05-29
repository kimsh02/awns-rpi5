extern "C" {
#include <gps.h>
}

#include <iostream>

#include "gps.hpp"

int main()
{
	GPSClient gps{};
	gps.connect();
	gps.startStream();

	GPSFix fix = gps.waitReadFix().value_or(GPSFix{ 0, 0 });
	std::cout << "Lat: " << fix.latitude << ", Lon: " << fix.longitude
		  << "\n";
	GPSFix fix1 = gps.waitReadFix().value_or(GPSFix{ 0, 0 });
	std::cout << "Lat: " << fix1.latitude << ", Lon: " << fix1.longitude
		  << "\n";
	GPSFix fix2 = gps.waitReadFix().value_or(GPSFix{ 0, 0 });
	std::cout << "Lat: " << fix2.latitude << ", Lon: " << fix2.longitude
		  << "\n";

	gps.stopStream();
	return 0;
}
