#include "navigator.hpp"

#include <cstdlib>
#include <filesystem>
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

/* Helper method to use ConcordeTSPSolver to parse CSV */
bool Navigator::readCSV(ConcordeTSPSolver &concorde)
{
	std::cout << "Enter waypoint CSV path: ";
	std::filesystem::path path{};
	std::cin >> path;
	/* Set CSV path */
	concorde.setCSVFile(std::move(expandTilde(path)));
	/* Read in waypoints from CSV */
	if (concorde.readCSV()) {
		/* If able to, return true */
		return true;
	}
	/* Else return false */
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
			/* If read was successful proceed */
			if (readCSV(concorde)) {
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

/* Helper function helping to expand tilde if user opts to use it in CSV path
   string */
std::filesystem::path Navigator::expandTilde(const std::filesystem::path &p)
{
	std::string s = p.string();
	if (!s.empty() && s[0] == '~') {
		const char *home = std::getenv("HOME");
		if (home) {
			s.replace(0, 1, home); // replace ~ with $HOME
			return std::filesystem::path(s);
		}
	}
	return p; // return as-is if no expansion needed
}

/* Helper method to set TSP directory for Concorde*/
bool Navigator::setTSPDir(ConcordeTSPSolver &concorde)
{
	std::cout << "Enter TSP directory: ";
	std::filesystem::path tsvDir{};
	std::cin >> tsvDir;
	tsvDir = expandTilde(tsvDir);
	if (checkValidDir(tsvDir)) {
		concorde.setTSPDir(std::move(tsvDir));
		return true;
	}
	return false;
}

/* Helper method to set solution directory for Concorde*/
bool Navigator::setSolDir(ConcordeTSPSolver &concorde)
{
	std::cout << "Enter solution directory: ";
	std::filesystem::path solDir{};
	std::cin >> solDir;
	solDir = expandTilde(solDir);
	if (checkValidDir(solDir)) {
		concorde.setSolDir(std::move(solDir));
		return true;
	}
	return false;
}

/* Helper method to check CSV directory */
bool Navigator::checkValidDir(std::filesystem::path &p)
{
	/* Check for valid directory path */
	if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) {
		return true;
	}
	return false;
}

/* CLI mode to solve directory of waypoints */
[[noreturn]] void Navigator::solve(void)
{
	/* Initialize ConcordeTSPSolver */
	ConcordeTSPSolver concorde{};
	/* Initialize CSV directory path */
	std::filesystem::path csvDir{};
	/* Get valid CSV directory */
	while (true) {
		std::cout << "Enter CSV waypoint directory: ";
		std::cin >> csvDir;
		csvDir = expandTilde(csvDir);
		/* Check for valid directory path */
		if (checkValidDir(csvDir)) {
			/* If valid, proceed */
			break;
		}
		/* Else ask user to retry */
		retryPrompt("CSV directory not valid.");
	}
	/* Set TSP directory */
	while (true) {
		if (setTSPDir(concorde)) {
			break;
		}
		retryPrompt("TSV directory not valid.");
	}
	/* Set solution directory */
	while (true) {
		if (setSolDir(concorde)) {
			break;
		}
		retryPrompt("Solution directory not valid.");
	}
	/* Helper method to solve CSV dir */
	solveCSVDir(concorde);

	std::exit(0);
}

/* Helper method to solve CSV dir */
void Navigator::solveCSVDir(ConcordeTSPSolver &)
{
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
