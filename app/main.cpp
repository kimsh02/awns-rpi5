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

	int ctr = 0;
	while (true) {
		GPSFix fix = gps.readFix().value_or(GPSFix{ 0, 0 });
		std::cout << "Lat: " << fix.latitude
			  << ", Lon: " << fix.longitude << "\n";
		ctr++;
		if (fix.latitude) {
			break;
		}
	}
	std::cout << ctr << " tries\n";

	gps.stopStream();
	return 0;
}
