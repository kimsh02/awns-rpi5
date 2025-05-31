#include "navigator.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "gps.hpp"

/* Log waypoint method to print waypoints to stdout */
void Navigator::logFix(GPSFix fix) noexcept
{
	std::cout << "[Latitude: " << fix.latitude
		  << ", Longitude: " << fix.longitude
		  << ", Bearing: " << fix.heading << "]\n";
}

/* Read CSV file to load in waypoints */
bool Navigator::readCSV(void)
{
	/* First, clear waypoints_ */
	waypoints_.clear();
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
	size_t lineNo	    = 0;
	size_t numWaypoints = 0;
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
			/* Add waypoint to waypoints_ */
			waypoints_.emplace_back(
				std::stod(lat),
				std::stod(lon),
				std::numeric_limits<double>::quiet_NaN());
			numWaypoints++;
			/* Print waypoint that was loaded */
			std::cout << "(" << numWaypoints << ") ";
			logFix(GPSFix{
				std::stod(lat),
				std::stod(lon),
				std::numeric_limits<double>::quiet_NaN() });
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
	/* Else print number of waypoints and return true */
	std::cout << numWaypoints << " waypoints loaded.\n\n";
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
				logFix(optFix ? *optFix : GPSFix{ 0, 0, 0 });
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

/* Print helper asking user to retry an action */
void Navigator::retryPrompt(const char *message) noexcept
{
	std::cout << message << " Press Enter to retry.";
	std::cout.flush();
	/* Discard any leftover characters on the current line (including '\n'): */
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	/* Now wait for the user to press ENTER (blocks on the next '\n'): */
	std::string dummy;
	std::getline(std::cin, dummy);
	std::cout << "\n";
}

Navigator::Navigator(int argc, const char **argv) noexcept : argc_{ argc },
							     argv_{ argv }
{
}

void Navigator::args(void)
{
	/* If incorrect number of args passed, default to help */
	if (argc_ != 2) {
		help();
	} else {
		std::string arg{ argv[1] };
		if (arg == "play") {
			if (argc == 3) {
				std::string userWord{ argv[2] };
				validateWord(userWord);
				setUserWordOfDay(userWord);
			}
			tileGrid.setWordOfDay(wordOfDay);
			std::cout
				<< "-----------------------------------------------------\n";
			play(true);
			std::cout
				<< "\n-----------------------------------------------------\n";
		} else if (arg == "peek" && argc == 2) {
			tileGrid.setWordOfDay(wordOfDay);
			peek();
		} else if (arg == "bm") {
			if (argc == 3) {
				std::string userOpener{ argv[2] };
				validateWord(userOpener);
				wordlePlayer.setUserOpener(userOpener);
			}
			benchmark(false);
		} else if (arg == "bmv") {
			if (argc == 3) {
				std::string userOpener{ argv[2] };
				validateWord(userOpener);
				wordlePlayer.setUserOpener(userOpener);
			}
			benchmark(true);
		} else {
			help();
		}
	}
}

[[noreturn]] void Navigator::solve(void)
{
	std::exit(0);
}

void Navigator::writeTSPFile(void)
{
}

void Navigator::runConcorde(void)
{
}

void Navigator::readSolution(void)
{
}

void Navigator::help(void) noexcept
{
}
