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

			/* Poll GPS 5 times to ensure connection */
			bool connected = false;
			for (size_t i = 0; i < 5; i++) {
				auto optFix{ gps.waitReadFix() };
				std::cout << "(" << i + 1 << "/5) ";
				logWaypoint(optFix ? *optFix :
						     GPSFix{ 0, 0, 0 });
				/* Check that last poll gives a fix */
				if (i == 4 && optFix) {
					std::cout
						<< "GPS connection successful.\n\n";
					connected = true;
				}
			}
			/* If GPS connection test was successful, proceed */
			if (connected) {
				break;
			}
			/* Else, ask user whether to retry connection */
			retryPrompt("GPS connection failed.");
		}
		/* Enter waypoint CSV path */
		while (true) {
			std::cout << "Enter waypoint CSV path: ";
			std::cin >> csv_path_;
			/* Read in waypoints from CSV */
			if (readCSV()) {
				break;
			}
			/* If reading CSV failed, prompt user to retry */
			retryPrompt("Reading CSV failed.");
		}

		/* Placeholder */
		gps.stopStream();
		std::exit(0);
	}
}

void Navigator::bestPath(void) noexcept
{
}

/* Print helper asking user to retry an action */
void Navigator::retryPrompt(const char *message) noexcept
{
	std::cout << message;
	std::cout << " Press enter to retry.";
	std::string dummy{};
	/* Discard any leftover characters up to the next newline */
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << "\n";
}
