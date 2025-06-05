#include "navigator.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "concorde.hpp"
#include "gps.hpp"

/* Log waypoint method to print waypoints to stdout */
void Navigator::logFix(const GPSFix &fix) noexcept
{
	std::cout << std::setprecision(6) << "[Latitude: " << fix.latitude
		  << ", Longitude: " << fix.longitude
		  << ", Bearing: " << fix.heading << "]\n";
}

/* Helper method to test GPS connection in hot loop */
bool Navigator::testGPSConnection(void)
{
	std::cout << "Testing GPS connection.\n";
	if (!gps_.connect()) {
		return false;
	}
	gps_.startStream();
	/* Poll GPS 6 times to ensure connection */
	for (size_t i = 0; i < 6; i++) {
		auto optFix{ gps_.waitReadFix() };
		std::cout << "(" << i + 1 << "/6) ";
		logFix(optFix ? *optFix : GPSFix{ 0, 0, 0 });
		/* Check that last poll gives a fix */
		if (i == 5 && optFix) {
			std::cout << "GPS connection successful.\n\n";
			return true;
		}
	}
	return false;
}

/* Helper method to use ConcordeTSPSolver to parse CSV */
bool Navigator::readCSV(void)
{
	std::cout << "Enter waypoint CSV path: ";
	std::filesystem::path path{};
	std::cin >> path;
	/* Set CSV path */
	concorde_.setCSVFile(std::move(expandTilde(path)));
	/* Read in waypoints from CSV */
	if (concorde_.readCSV()) {
		/* If able to, return true */
		return true;
	}
	/* Else return false */
	return false;
}

/* Hot loop to run navigation system */
void Navigator::run(void)
{
	while (true) {
		/* Test GPS connection */
		gpspoll(false);
		/* Enter waypoint CSV path */
		while (true) {
			/* If read was successful proceed */
			if (readCSV()) {
				break;
			}
			/* If reading CSV failed, prompt user to retry */
			retryPrompt("Reading CSV failed.");
		}
	}
}

/* Stop GPS stream connection */
void Navigator::stop(void)
{
	gps_.stopStream();
}

/* Print helper asking user to retry an action */
void Navigator::retryPrompt(const char *message) noexcept
{
	std::cout << message << " Press Enter to retry.";
	std::cout.flush();

	// Grab the stream buffer for cin:
	auto *buf = std::cin.rdbuf();
	// As long as there is at least one character already buffered,
	// and that character is '\n', consume it:
	while (buf->in_avail() > 0 && std::cin.peek() == '\n') {
		std::cin.get(); // discard exactly one '\n'
	}

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

/* GPS poll, if exit is true then exit program */
void Navigator::gpspoll(bool exit)
{
	/* Test GPS connection */
	while (true) {
		/* If GPS connection test was successful, proceed */
		if (testGPSConnection()) {
			break;
		}
		/* Else, ask user whether to retry connection */
		retryPrompt("GPS connection failed.");
	}
	if (exit) {
		std::exit(0);
	}
}

/* Starting point of Navigator that parses user args */
void Navigator::start(void) noexcept
{
	/* If incorrect number of args passed, default to help */
	if (argc_ != 2) {
		help();
	} else {
		std::string argStr{ argv_[1] };
		if (argStr == "gpspoll") { /* Go to gpspoll */
			gpspoll(true);
		} else if (argStr == "run") { /* Go to run */
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
			s.replace(0, 1,
				  home); // replace ~ with $HOME
			return std::filesystem::path(s);
		}
	}
	return p; // return as-is if no expansion needed
}

/* Helper method to set TSP directory for Concorde*/
bool Navigator::setCSVDir(void)
{
	std::cout << "Enter CSV waypoint directory: ";
	std::filesystem::path csvDir{};
	std::cin >> csvDir;
	csvDir = expandTilde(csvDir);
	/* Check for valid directory path */
	if (checkValidDir(csvDir)) {
		concorde_.setCSVDir(std::move(csvDir));
		printPath(csvDir);
		return true;
	}
	return false;
}

/* Helper method to set TSP directory for Concorde*/
bool Navigator::setTSPDir(void)
{
	std::cout << "Enter TSP directory: ";
	std::filesystem::path tspDir{};
	std::cin >> tspDir;
	tspDir = expandTilde(tspDir);
	if (checkValidDir(tspDir)) {
		concorde_.setTSPDir(std::move(tspDir));
		printPath(tspDir);
		return true;
	}
	return false;
}

/* Helper method to set solution directory for Concorde*/
bool Navigator::setSolDir(void)
{
	std::cout << "Enter solution directory: ";
	std::filesystem::path solDir{};
	std::cin >> solDir;
	solDir = expandTilde(solDir);
	if (checkValidDir(solDir)) {
		concorde_.setSolDir(std::move(solDir));
		printPath(solDir);
		return true;
	}
	return false;
}

/* Print path helper method */
void Navigator::printPath(const std::filesystem::path &p)
{
	std::cout << std::filesystem::absolute(p) << " found.\n\n";
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

/* Helper method to generate solutions from CSV directory */
void Navigator::makeSolutions(void)
{
	/* Iterate through every CSV file in CSV directory */
	std::size_t solCtr{ 0 };
	std::size_t iterCtr{ 0 };
	for (auto const &entry :
	     std::filesystem::directory_iterator(concorde_.getCSVDir())) {
		/* Increment iteration ctr */
		iterCtr++;
		/* If not regular file, skip */
		if (!entry.is_regular_file())
			continue;
		auto path = entry.path();
		/* If file not CSV, skip */
		if (path.extension() != ".csv")
			continue;
		/* Set CSV file in ConcordeTSPSolver */
		concorde_.setCSVFile(std::move(path));
		/* Read CSV file */
		if (!concorde_.readCSV()) {
			/* If CSV unreadable, skip */
			continue;
		}
		/* Write TSP file */
		concorde_.writeTSPFile();
		/* Run Concorde on TSP file */
		/* Measure time it takes to solve TSP file */
		auto start = std::chrono::steady_clock::now();
		/* Invoke Concorde to solve TSP file */
		concorde_.solveTSP();
		auto end = std::chrono::steady_clock::now();
		auto duration =
			std::chrono::duration_cast<std::chrono::microseconds>(
				end - start);
		std::cout << "Solved in " << duration << " microseonds.\n";

		/* Increment solCtr to track number of solutions generated */
		solCtr++;
	}
	/* If no solution files created, print notice */
	if (!solCtr) {
		std::cerr
			<< "Error: No solution files were able to be created.\n";
	} else {
		/* Else print number of CSVs solved */
		std::cout << solCtr << "/" << iterCtr << " CSV files solved.\n";
	}
}

/* CLI mode to solve directory of waypoints */
[[noreturn]] void Navigator::solve(void)
{
	/* Test GPS connection */
	gpspoll(false);
	/* Set valid CSV directory */
	while (true) {
		if (setCSVDir()) {
			break;
		}
		retryPrompt("CSV directory not valid.");
	}
	/* Set TSP directory */
	while (true) {
		if (setTSPDir()) {
			break;
		}
		retryPrompt("TSV directory not valid.");
	}
	/* Set solution directory */
	while (true) {
		if (setSolDir()) {
			break;
		}
		retryPrompt("Solution directory not valid.");
	}
	/* Make solutions from CSV files */
	makeSolutions();
	std::exit(0);
}

/* User help print */
[[noreturn]] void Navigator::help(void) noexcept
{
	std::cout
		<< "Usage: " << prog_ << " COMMAND\n\n"
		<< "Autonomous waypoint navigation system for a mobile platform using Raspberry Pi 5\n\n"
		<< "Commands:\n"
		<< "  gpspoll        Poll GPS to get a reading\n"
		<< "  run            Use GPS data to guide platform along a predefined series of static waypoints and output logs\n"
		<< "  solve          Use Concorde TSP to solve directory of CSV waypoint files and output solutions as plotted graphs\n"
		<< "  help           Show this help message and exit\n"
		<< "\nExamples:\n"
		<< "  " << prog_ << " run\n"
		<< "  " << prog_ << " solve\n";
	std::exit(0);
}
