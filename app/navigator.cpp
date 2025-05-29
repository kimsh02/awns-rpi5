#include "navigator.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "gps.hpp"

/* Log waypoint method to print waypoints to stdout */
void Navigator::logWaypoint(GPSFix fix) noexcept
{
	std::cout << "Lat: " << fix.latitude << ", Lon: " << fix.longitude
		  << ", Heading: " << fix.heading << "\n";
}

/* Read CSV file for waypoints */
bool Navigator::readCSV(void)
{
	std::ifstream file;
	/* Catch invalid file path */
	file.open(csv_path_);
	if (!file) {
		std::cerr << "Error opening file '" << csv_path_ << "'.\n";
		return false;
	}
	std::string line;
	/* Skip header */
	/* Note: CSV must have a header labeling latitude and longitude and in
	   that order */
	std::getline(file, line);
	size_t lineNo = 0;
	/* Read CSV */
	while (std::getline(file, line)) {
		lineNo++;
		std::istringstream ss(line);
		std::string	   lat, lon;
		/* Skip malformed lines */
		if (!std::getline(ss, lat, ',') ||
		    !std::getline(ss, lon, ',')) {
			std::cerr << "Error: Line " << lineNo
				  << " malformed, skipping.\n";
			continue;
		}
		/* Skip blank fields */
		if (lat.empty() || lon.empty()) {
			std::cerr << "Error: Line " << lineNo
				  << " contains blank fields, skipping.\n";
			continue;
		}
		try {
			waypoints_.emplace_back(std::stod(lat), std::stod(lon));
		} catch (const std::exception &) {
			/* Catch malformed numbers */
			std::cerr << "Error: Line " << lineNo
				  << " contains malformed number, skipping.\n";
			continue;
		}
	}
	/* If reader was unable to add any waypoints for whatever reason */
	if (waypoints_.empty()) {
		std::cerr << "Error: Unable to add any waypoints.\n";
		return false;
	}
	return true;
}

/* Hot loop to run navigation system */
[[noreturn]] void Navigator::run(void)
{
	while (true) {
		GPSClient gps{};
		/* Test GPS connection */
		while (true) {
			std::cout << "Testing GPS connection.\n";
			gps.connect();
			gps.startStream();
			auto optFix{ gps.waitReadFix() };
			/* If a fix was received, print and break */
			if (optFix) {
				logWaypoint(*optFix);
				std::cout << "GPS connection successful.\n\n";
				break;
			}
			/* Else, ask user whether to retry */
			std::cout
				<< "GPS connection failed. A few attempts may be necessary if device was recently booted up. Press enter to retry.";
			std::string dummy{};
			std::getline(std::cin, dummy);
			std::cout << "\n";
		}
		/* Enter waypoint CSV path */
		while (true) {
			std::cout << "Enter waypoint CSV path: ";
			std::cin >> csv_path_;
			if (readCSV()) {
				break;
			}
		}

		/* Placeholder */
		gps.stopStream();
		std::exit(0);
	}
}

void Navigator::bestPath(void) noexcept
{
}
