extern "C" {
#include <gps.h>
}

#include <cerrno>
#include <cstdlib>
#include <iostream>

int main()
{
	gps_data_t gps_data{};
	// connect over the UNIX socket
	if (gps_open(nullptr, "/var/run/gpsd.sock", &gps_data) != 0) {
		std::cerr << "gps_open failed: " << gps_errstr(errno) << "\n";
		return EXIT_FAILURE;
	}

	gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, nullptr);

	while (gps_waiting(&gps_data, 5000000)) {
		if (gps_read(&gps_data, nullptr, 0) < 0) {
			std::cerr << "gps_read error: " << gps_errstr(errno)
				  << "\n";
			break;
		}
		if (gps_data.fix.mode >= MODE_2D) {
			std::cout << "Lat: " << gps_data.fix.latitude
				  << ", Lon: " << gps_data.fix.longitude
				  << ", Alt: " << gps_data.fix.altitude << "\n";
			break;
		}
	}

	gps_stream(&gps_data, WATCH_DISABLE, nullptr);
	gps_close(&gps_data);
	return EXIT_SUCCESS;
}
