#include "navigator.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <iostream>
#include <numbers>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "concorde.hpp"
#include "gps.hpp"

/* Get navigation output for downstream controller  */
/* If
   - navigator is not ready from start(),
   - proximity radius has not been set,
   - not able to obtain GPS reading,
   - or the navigation has successfully completed the tour,
     this returns null,
   else
   - return JSON of navigation output */
std::optional<json> Navigator::getOutput(void)
{
	/* If navigator not ready, or proximity radius not set, return null */
	if (!ready_) {
		std::cerr << "Error: please invoke start() first.\n";
		return std::nullopt;
	} else if (!proximityRadius_) {
		std::cerr << "error: please set proximity radius.\n";
		return std::nullopt;
	}
	/* If simulation velocity is set, predict system position */
	if (simulationVelocity_) {
		/* Call helper method for simulation velocity output */
		return simulationVelocityOutput();
	} else { /* Generate output based on GPS reading */
		 /* Call helper method for GPS reading output */
		return gpsOutput();
	}
}

/* Helper method for GPS output */
std::optional<json> Navigator::gpsOutput(void)
{
	/* Get GPS reading */
	auto optFix{ gps_.waitReadFix() };
	/* If can't get GPS reading, return null */
	if (!optFix) {
		return std::nullopt;
	}
	/* Get current position */
	GPSFix fix{ *optFix };
	/* Update 'currPos_' */
	currPos_.first	= fix.latitude;
	currPos_.second = fix.longitude;
	/* Get next destination */
	auto optDest{ getDest() };
	if (!optDest) { /* If route finished, return null */
		return std::nullopt;
	}
	dest_ = *optDest;
	/* Calculate direction to start heading */
	bearing_ = calculateBearing(currPos_, dest_);
	/* Return JSON ouput */
	auto tm{ localTime() };
	json j{
		{ "gps_position",
		  { { "latitude", fix.latitude },
		    { "longitude", fix.longitude } }   },
		{	  "bearing",	     bearing_ },
		{  "destination",
		  { { "latitude", dest_.first },
		    { "longitude", dest_.second } }    },
		{	  "timestamp",
		  { { "year", tm.tm_year },
		    { "month", tm.tm_mon + 1 },
		    { "day", tm.tm_mday },
		    { "hour", tm.tm_hour },
		    { "minute", tm.tm_min },
		    { "second", tm.tm_sec } }	      }
	};
	/* Print JSON and return */
	logPrint(j.dump(2), false);
	return j;
}

/* Helper method for simulation velocity output */
std::optional<json> Navigator::simulationVelocityOutput(void)
{
	/* Time GPS reading */
	auto start{ std::chrono::steady_clock::now() };
	auto optFix{ gps_.waitReadFix() };
	auto end{ std::chrono::steady_clock::now() };
	auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(
		end - start) };
	/* If can't get GPS reading, return null */
	if (!optFix) {
		return std::nullopt;
	}
	/* Init GPS fix */
	auto fix{ *optFix };
	/* If vehicle is in motion, calculate predicted position */
	if (inMotion_) {
		/* Update currPos */
		currPos_ =
			computeNewPosition(dest_, elapsed.count() / 1000000.0);
	} else { /* Vehicle is set to start moving */
		inMotion_ = true;
	}
	/* Get next destination */
	auto optDest{ getDest() };
	if (!optDest) { /* If route finished, return null */
		return std::nullopt;
	}
	dest_ = *optDest;
	/* Calculate direction to start heading */
	bearing_ = calculateBearing(currPos_, dest_);
	/* Return JSON ouput */
	auto tm{ localTime() };
	json j{
		{ "sim_position",
		  { { "latitude", currPos_.first },
		    { "longitude", currPos_.second } }   },
		{ "gps_position",
		  { { "latitude", fix.latitude },
		    { "longitude", fix.longitude } }     },
		{	  "velocity",  simulationVelocity_ },
		{	  "bearing",	     bearing_ },
		{  "destination",
		  { { "latitude", dest_.first },
		    { "longitude", dest_.second } }	    },
		{	  "timestamp",
		  { { "year", tm.tm_year },
		    { "month", tm.tm_mon + 1 },
		    { "day", tm.tm_mday },
		    { "hour", tm.tm_hour },
		    { "minute", tm.tm_min },
		    { "second", tm.tm_sec } }	      }
	};
	/* Print JSON and return */
	logPrint(j.dump(2), false);
	return j;
}

/* Helper method to calculate bearing */
// Compute initial bearing (degrees from North) from 'current' to
// 'destination' Returns a value in [0,360)
/* calculateBearing uses the “forward azimuth” formula on a spherical Earth */
double Navigator::calculateBearing(
	const std::pair<double, double> &current,
	const std::pair<double, double> &destination) noexcept
{
	double φ1 = deg2rad(current.first);
	double λ1 = deg2rad(current.second);
	double φ2 = deg2rad(destination.first);
	double λ2 = deg2rad(destination.second);
	double Δλ = λ2 - λ1;
	double y  = std::sin(Δλ) * std::cos(φ2);
	double x  = std::cos(φ1) * std::sin(φ2) -
		   std::sin(φ1) * std::cos(φ2) * std::cos(Δλ);

	double θ   = std::atan2(y, x);	      // radians
	double deg = rad2deg(θ);	      // convert to degrees
	return std::fmod(deg + 360.0, 360.0); // normalize to [0,360)
}

/* Helper method to compute new position */
// Compute new position after moving at 'simulationVelocity_' (m/s) along
// 'bearing_' for 'timeSec' seconds.  Returns { latitude, longitude } in
// degrees.
/* computeNewPosition uses the inverse great‐circle formula (based on haversine
   geometry) */
std::pair<double, double>
Navigator::computeNewPosition(const std::pair<double, double> &initial,
			      double			       timeSec) noexcept
{
	// distance travelled along great‐circle (meters) → central angle (radians)
	double dist = simulationVelocity_ * timeSec;
	double δ    = dist / earthRadius_;
	double θ    = deg2rad(bearing_);
	double φ1   = deg2rad(initial.first);
	double λ1   = deg2rad(initial.second);
	// compute φ2 (lat₂) and λ2 (lon₂)
	double φ2 = std::asin(std::sin(φ1) * std::cos(δ) +
			      std::cos(φ1) * std::sin(δ) * std::cos(θ));
	double λ2 = λ1 + std::atan2(std::sin(θ) * std::sin(δ) * std::cos(φ1),
				    std::cos(δ) - std::sin(φ1) * std::sin(φ2));
	// convert back to degrees and normalize longitude to (–180, +180]
	double lat2 = rad2deg(φ2);
	double lon2 = std::fmod(rad2deg(λ2) + 540.0, 360.0) - 180.0;
	return { lat2, lon2 };
}

/* Helper method to check whether destination has been reached */
/// Returns true if 'current' is within proximityRadius_ of 'destination'.
/// current      {latitude, longitude} in degrees
/// destination  {latitude, longitude} in degrees
/// return true if distance ≤ proximityRadius_
bool Navigator::waypointReached(
	const std::pair<double, double> &current,
	const std::pair<double, double> &destination) const noexcept
{
	// 1. Convert degrees → radians
	constexpr double degToRad = std::numbers::pi / 180.0;
	const double	 lat1	  = current.first * degToRad;
	const double	 lon1	  = current.second * degToRad;
	const double	 lat2	  = destination.first * degToRad;
	const double	 lon2	  = destination.second * degToRad;
	// 2. Haversine formula
	const double dLat     = lat2 - lat1;
	const double dLon     = lon2 - lon1;
	const double sinDLat2 = std::sin(dLat / 2);
	const double sinDLon2 = std::sin(dLon / 2);
	const double a	      = sinDLat2 * sinDLat2 +
			 std::cos(lat1) * std::cos(lat2) * sinDLon2 * sinDLon2;
	const double c = 2 * std::asin(std::sqrt(a));
	// 3. Distance in meters
	const double distance = earthRadius_ * c;
	// 4. Compare against your proximity radius
	return distance <= proximityRadius_;
}

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

/* Helper method to get next destination */
std::optional<std::pair<double, double> > Navigator::getDest(void)
{
	/* If system has not reached the current destiation, return it */
	if (!waypointReached(currPos_, tour_[nextDest_])) {
		return tour_[nextDest_];
	} else { /* Else get next dest */
		/* Print destination has been reached */
		std::ostringstream oss{};
		oss << "Waypoint reached: " << logCoordinates(tour_[nextDest_]);
		logPrint(oss.str(), true);
		/* If nextDest_ is 0, then tour is over and return null */
		if (!nextDest_) {
			logPrint("Navigation has completed.", true);
			return std::nullopt;
		}
		/* Else, return next dest */
		nextDest_ = (nextDest_ + 1) % tour_.size();
		return tour_[nextDest_];
	}
}

/* Helper method to get local time */
std::tm Navigator::localTime(void)
{
	using namespace std::chrono;
	auto	    now = system_clock::now();
	std::time_t t	= system_clock::to_time_t(now);
	return *std::localtime(&t); // for UTC use std::gmtime
}

/* Helper method to add timestamp to print */
std::string Navigator::logWithTimestamp(const std::string &message)
{
	auto		   tm{ localTime() };
	std::ostringstream oss{};
	oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
	    << "] (System Message) " << message;
	return oss.str();
}

/* Helper method to print to stdout and log file */
void Navigator::logPrint(const std::string &message, bool timeStamp)
{
	if (logFile_.is_open()) { /* If log enabled */
		if (timeStamp) {
			logFile_ << logWithTimestamp(message) << "\n";
		} else {
			logFile_ << message << "\n";
		}
	}
	if (timeStamp) {
		std::cout << logWithTimestamp(message) << "\n";
	} else {
		std::cout << message << "\n";
	}
}

/* Helper method to log coordinates */
std::string Navigator::logCoordinates(const std::pair<double, double> &c)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(4) << "[Latitude: " << c.first
	    << ", Longitude: " << c.second << "]";
	return oss.str();
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
	std::filesystem::path csvFile{ expandTilde(path) };
	concorde_.setCSVFile(csvFile);
	/* Read in waypoints from CSV */
	if (concorde_.readCSV()) {
		/* If able to, return true */
		std::cout << "\n";
		/* Set csvFile_ */
		csvFile_ = csvFile;
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
	/* Setup for navigation output */
	setupForNavOutput();
	/* Print ready output */
	std::cout
		<< "\033[1;32m"
		<< "Optimal tour has been calculated. Ready to provide navigation output.\n\n"
		<< "\033[0m";
}

/* Helper method to setup for navigation output */
void Navigator::setupForNavOutput(void)
{
	/* Set current position of system */
	currPos_ = tour_.at(0);
	/* Navigator is ready */
	ready_ = true;
	/* Create and open log file, if logging */
	if (!logDir_.empty()) {
		std::filesystem::path logFile{
			logDir_ / (csvFile_.stem().string() + ".log")
		};
		logFile_.open(logFile);
		if (!logFile_.is_open()) {
			throw std::runtime_error("Failed to open log file.");
		}
	}
}

/* Stop GPS stream connection */
void Navigator::stop(void)
{
	/* Stop GPS stream */
	gps_.stopStream();
	/* Stop log file stream if open */
	if (logFile_.is_open()) {
		logFile_.close();
	}
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
		concorde_.setCSVDir(csvDir);
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
		concorde_.setTSPDir((tspDir));
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
		concorde_.setSolDir(solDir);
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
		concorde_.setGraphDir(graphDir);
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
