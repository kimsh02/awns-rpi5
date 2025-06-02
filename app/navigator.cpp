#include "navigator.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

#include "concorde.hpp"
#include "gps.hpp"

/* Log waypoint method to print waypoints to stdout */
void Navigator::logFix(const GPSFix &fix) noexcept
{
	std::cout << "[Latitude: " << fix.latitude
		  << ", Longitude: " << fix.longitude
		  << ", Bearing: " << fix.heading << "]\n";
}

/* Helper method to test GPS connection in hot loop */
bool Navigator::testGPSConnection(GPSClient &gps)
{
	std::cout << "Testing GPS connection.\n";
	gps.connect();
	gps.startStream();

	/* Poll GPS 5 times to ensure connection */
	for (size_t i = 0; i < 5; i++) {
		auto optFix{ gps.waitReadFix() };
		std::cout << "(" << i + 1 << "/5) ";
		logFix(optFix ? *optFix : GPSFix{ 0, 0, 0 });
		/* Check that last poll gives a fix */
		if (i == 4 && optFix) {
			std::cout << "GPS connection successful.\n\n";
			return true;
		}
	}
	return false;
}

/* Hot loop to run navigation system */
[[noreturn]] void Navigator::run(void)
{
	while (true) {
		/* Initialize GPS client */
		GPSClient gps{};
		/* Test GPS connection */
		while (true) {
			/* If GPS connection test was successful, proceed */
			if (testGPSConnection(gps)) {
				break;
			}
			/* Else, ask user whether to retry connection */
			retryPrompt("GPS connection failed.");
		}
		/* Initialize Concorde TSP solver */
		ConcordeTSPSolver concorde{};
		/* Enter waypoint CSV path */
		while (true) {
			std::cout << "Enter waypoint CSV path: ";
			std::filesystem::path path{};
			std::cin >> path;
			/* Set CSV path */
			concorde.setCSVFile(std::move(path));
			/* Read in waypoints from CSV */
			if (concorde.readCSV()) {
				/*If read was successful proceed */
				break;
			}
			/* If reading CSV failed, prompt user to retry */
			retryPrompt("Reading CSV failed.");
		}

		/* Stop GPS stream and exit program */
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

/* Constructor */
Navigator::Navigator(int argc, const char **argv) noexcept : prog_{ *argv },
							     argc_{ argc },
							     argv_{ argv }
{
}

/* Starting point of Navigator that parses user args */
void Navigator::start(void) noexcept
{
	/* If incorrect number of args passed, default to help */
	if (argc_ != 2) {
		help();
	} else {
		std::string argStr{ argv_[1] };
		if (argStr == "run") { /* Go to run */
			run();
		} else if (argStr == "solve") { /* Go to solve  */
			solve();
		} else { /* Any other string is invalid so default to help */
			help();
		}
	}
}

/* CLI mode to solve directory of waypoints */
[[noreturn]] void Navigator::solve(void)
{
	std::cout << "Enter CSV waypoint directory: ";
	std::string csvDir;
	std::cin >> csvDir;

	std::exit(0);
}

/* User help print */
[[noreturn]] void Navigator::help(void) noexcept
{
	std::cout
		<< "Usage: " << prog_ << " COMMAND\n\n"
		<< "Autonomous waypoint navigation for a mobile platform using Raspberry Pi 5"
		<< "Commands:\n"
		<< "  run            Use GPS data to guide platform along a predefined series of static waypoints and output logs\n"
		<< "  solve          Use Concorde TSP to solve directory of CSV waypoint files and output solutions as plotted graphs\n"
		<< "  help           Show this help message and exit\n"
		<< "\nExamples:\n"
		<< "  " << prog_ << " run\n"
		<< "  " << prog_ << " solve\n";
	std::exit(0);
}
