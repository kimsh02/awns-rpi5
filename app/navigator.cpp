#include "navigator.hpp"

#include <fstream>
#include <iostream>

#include "gps.hpp"

/* Log waypoint method to print waypoints to stdout */
void Navigator::logWaypoint(GPSFix fix) noexcept
{
	std::cout << "Lat: " << fix.latitude << ", Lon: " << fix.longitude
		  << "\n";
}

/* Read CSV file for waypoints */
// bool Navigator::readCSV(void)
// {
// 	std::ifstream f{ csv_path_ };
// 	std::string   line;
// }

/* Hot loop to run navigation system */
void Navigator::run(void)
{
	while (true) {
		/* Enter waypoint CSV file path */
		while (true) {
			std::cout << "Enter waypoint CSV file path: ";
			std::cin >> csv_path_;
			if (readCSV()) {
				break;
			}
			std::cout << "Invalid path.\n";
		}
	}
}
