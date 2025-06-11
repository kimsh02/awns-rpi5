#include "navigator.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "concorde.hpp"
#include "gps.hpp"

// /* Get navigation output based on simulated downstream motor controller */
// /* Takes as input the velocity of simulated motor */
// /* If navigator is not ready from start(), or navigation has completed, this
//    returns null, else return JSON of navigation output */
// std::optional<json> Navigator::getOutput(double velocity)
// {
// 	/* Get and time GPS reading */
// 	auto start{ std::chrono::steady_clock::now() };
// 	auto optFix{ gps_.waitReadFix() };
// 	auto end{ std::chrono::steady_clock::now() };
// 	auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(
// 		end - start) };

// 	/* TODO: Get predicted position */
// 	std::pair<double, double> predictedLoc{};
// }

// /* Get navigation output for downstream controller  */
// /* If navigator is not ready from start(), or proximity radius has not been set,
//    or the navigation has successfully completed the tour, this returns null,
//    else return JSON of navigation output */
// std::optional<json> Navigator::getOutput(void)
// {
// 	/* If navigator not ready, or proximity radius not set, return null */
// 	if (!ready_) {
// 		std::cerr << "Error: please invoke start() first.\n";
// 		return std::nullopt;
// 	} else if (!proximityradius_) {
// 		std::cerr << "error: please set proximity radius.\n";
// 		return std::nullopt;
// 	}
// 	/* Get GPS reading */
// 	auto optFix{ gps_.waitReadFix() };
// 	/* If can't get GPS reading, return null */
// 	if (!optFix) {
// 		return std::nullopt;
// 	}
// 	/* If simulation velocity is set, get */
// 	if (simulationVelocity_) {

// 	}
// 	/* Get current position */
// 	GPSFix			  currFix{ *optFix };
// 	std::pair<double, double> gpsLoc{ currFix.latitude, currFix.longitude };
// 	/* Get destination */
// 	std::pair<double, double> dest{ tour_[nextDest()] };
// 	/* Check whether destination has been reached */

// 	/* Compute direction to head */

// 	/* speed, direction */
// /* If logDir_ set */
// }

/* Helper method to check whether destination has been reached */
bool Navigator::waypointReached(const std::pair<double, double> &curr,
				const std::pair<double, double> &dest) noexcept
{
	double latDiff{ std::abs(curr.first) - std::abs(dest.first) };
	double lonDiff{ std::abs(curr.second) - std::abs(dest.second) };
	return (latDiff <= proximityRadius_) && (lonDiff <= proximityRadius_);
}

/* Helper method to get predicted location of system */
// std::pair<double, double> Navigator::getPredictedLoc(void)
// {
// 	if (!(speed || direction))
// }

/* Setter for proximity radius threshold for waypont arrival */
/* Cannot be set to less than 1.0 */
void Navigator::setProximityRadius(double r) noexcept
{
	if (r < 1.0) {
		proximityRadius_ = 1.0;
	} else {
		proximityRadius_ = r;
	}
}

/* Setter for velocity of simulated downstream controller */
/* Cannot be set to a negative value */
void Navigator::setSimulationVelocity(double v) noexcept
{
	if (v < 0.0) {
		simulationVelocity_ = 0.0;
	} else {
		simulationVelocity_ = v;
	}
}

/* Helper method to get index of next destination */
std::size_t Navigator::nextDest(void)
{
	/* If nextDest_ is index 0, then tour is over and keep returning 0 */
	if (!nextDest_) {
		return nextDest_;
	}
	/* Else, return index of next destination */
	return nextDest_++ % tour_.size();
}

/* Log waypoint method to print waypoints to stdout */
void Navigator::logFix(const GPSFix &fix) noexcept
{
	std::cout << std::fixed << std::setprecision(4)
		  << "[Latitude: " << fix.latitude
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
	/* Poll GPS 5 times to ensure connection */
	std::size_t tries = 5;
	for (size_t i = 0; i < tries; i++) {
		auto optFix{ gps_.waitReadFix() };
		std::cout << "(" << i + 1 << "/" << tries << ") ";
		logFix(optFix ? *optFix : GPSFix{ 0, 0, 0 });
		/* Check that last poll gives a fix */
		if (i == tries - 1 && optFix) {
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
		std::cout << "\n";
		return true;
	}
	/* Else return false */
	return false;
}

/* Hot loop to run navigation system */
void Navigator::run(void)
{
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
	/* Set directories for Concorde */
	setDirectories(false, true);
	/* Read and generate solution from TSP file */
	concordeTSP();
	/* Navigator is ready */
	ready_ = true;
	std::cout
		<< "\033[1;32m"
		<< "Optimal tour has been calculated. Ready to provide navigation output.\n"
		<< "\033[0m";
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
	// 1) Discard everything up to and including the next '\n':
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	// 2) Now wait for the user to press Enter:
	std::cin.get(); // blocks until one more '\n' arrives
	std::cout << "\n";
}

/* Constructor */
Navigator::Navigator(int argc, const char **argv) noexcept
	: prog_{ *argv },
	  argc_{ argc },
	  argv_{ argv },
	  ready_{ false },
	  tour_{ concorde_.getTour() },
	  nextDest_{ 1 },
	  proximityRadius_{ 0 },
	  simulationVelocity_{ 0 }
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

/* Helper for setLogDir */
bool Navigator::setLogDirHelper(void)
{
	std::cout << "Enter log directory: ";
	std::cin >> logDir_;
	logDir_ = expandTilde(logDir_);
	if (checkValidDir(logDir_)) {
		printPath(logDir_);
		return true;
	}
	return false;
}

/* Helper method to set log directory for Navigator */
void Navigator::setLogDir(void)
{
	while (true) {
		std::cout << "Log controller output to file? [y/n]: ";
		std::string res{};
		std::cin >> res;
		char r{ static_cast<char>(std::tolower(res.at(0))) };
		if (r == 'y') {
			while (true) {
				if (setLogDirHelper()) {
					break;
				}
				retryPrompt("Log directory not valid.");
			}
			break;
		} else if (r == 'n') {
			std::cout << "\n";
			break;
		}
	}
}

/* Helper method to set graph directory for Concorde*/
bool Navigator::setGraphDir(void)
{
	std::cout << "Enter graph directory: ";
	std::filesystem::path graphDir{};
	std::cin >> graphDir;
	graphDir = expandTilde(graphDir);
	if (checkValidDir(graphDir)) {
		concorde_.setGraphDir(std::move(graphDir));
		printPath(graphDir);
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
bool Navigator::checkValidDir(const std::filesystem::path &p)
{
	/* Check for valid directory path */
	if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) {
		return true;
	}
	return false;
}

/* Helper method to invoke and time Concorde solving TSP */
void Navigator::solveTSPMeasureTime(void)
{
	/* Measure time it takes to solve TSP file */
	auto start{ std::chrono::steady_clock::now() };
	/* Invoke Concorde to solve TSP file */
	concorde_.solveTSP();
	auto end{ std::chrono::steady_clock::now() };
	auto duration{ std::chrono::duration_cast<std::chrono::microseconds>(
		end - start) };
	std::cout << "Solved optimal tour order in " << duration << ".\n";
}

/* Helper method to read and generate solution from CSV file */
void Navigator::concordeTSP(void)
{
	/* Write TSP file */
	concorde_.writeTSPFile();
	/* Run Concorde on TSP file */
	solveTSPMeasureTime();
	/* Read in TSP solution back into memory and set tour_ */
	concorde_.readTSPSolution();
	/* Plot graph in Python */
	concorde_.plotTSPSolution();
}

/* Helper method to generate solutions from CSV directory */
void Navigator::makeSolutions(void)
{
	/* Iterate through every CSV file in CSV directory */
	std::size_t solCtr{ 0 };
	for (auto const &entry :
	     std::filesystem::directory_iterator(concorde_.getCSVDir())) {
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
			/* if csv unreadable, skip */
			continue;
		}
		/* helper method to read and generate solution from CSV file */
		concordeTSP();
		/* Increment solCtr to track number of solutions generated */
		solCtr++;
		std::cout << "\n";
	}
	/* If no solution files created, print notice */
	if (!solCtr) {
		std::cerr
			<< "Error: No solution files were able to be created.\n";
	} else {
		/* Else print number of CSVs solved */
		std::cout << "\033[1;32m" << solCtr << " routes solved total.\n"
			  << "\033[0m";
	}
}

/* Helper method to set necessary directories for Concorde */
/* First param pass true if asking for CSV directory, false otherwise */
/* Second param pass tue if navigator should log controller output, false
   otherwise */
void Navigator::setDirectories(bool csvDir, bool logDir)
{
	if (csvDir) {
		/* Set valid CSV directory */
		while (true) {
			if (setCSVDir()) {
				break;
			}
			retryPrompt("CSV directory not valid.");
		}
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
	/* Set graph directory */
	while (true) {
		if (setGraphDir()) {
			break;
		}
		retryPrompt("Graph directory not valid.");
	}
	/* Optional: set log directory */
	if (logDir) {
		setLogDir();
	}
}

/* CLI mode to solve directory of waypoints */
[[noreturn]] void Navigator::solve(void)
{
	/* Set directories for Concorde */
	setDirectories(true, false);
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
