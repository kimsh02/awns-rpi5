extern "C" {
#include <gps.h> // GPS client library
}

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream> // std::cout

int main()
{
	gps_data_t gps_data{}; // struct to hold GPS data
	if (gps_open(nullptr, "/var/run/gpsd.sock", &gps_data) !=
	    0) // connect to gpsd
		return EXIT_FAILURE;

	gps_stream(&gps_data,
		   WATCH_ENABLE | WATCH_JSON,
		   nullptr); // start streaming JSON data

	while (gps_waiting(&gps_data, 5000000)) { // wait up to 5s for data
		if (gps_read(&gps_data, nullptr, 0) < 0) // read next packet
			break;
		if (gps_data.fix.mode >= MODE_2D) { // check for valid 2D+ fix
			std::cout << "Latitude: " << gps_data.fix.latitude
				  << ", Longitude: " << gps_data.fix.longitude
				  << ", Altitude: " << gps_data.fix.altitude
				  << "\n"; // output coordinates
			break;
		}
	}

	gps_stream(&gps_data, WATCH_DISABLE, nullptr); // stop streaming
	gps_close(&gps_data);			       // close connection
	return EXIT_SUCCESS;
}
