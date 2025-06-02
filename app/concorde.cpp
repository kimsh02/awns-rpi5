#include "concorde.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

/* Writes out TSP file from waypoints to be used by Concorde executable*/
void ConcordeTSPSolver::writeTSPFile(void)
{
	std::string base{ csvFile_.stem().string() };
}

/* Calls on Concorde to solve the TSP file and write out solution file */
void ConcordeTSPSolver::solveTSP(void)
{
}

/* Read CSV file to load in waypoints */
bool ConcordeTSPSolver::readCSV(void)
{
	/* First, clear waypoints_ */
	waypoints_.clear();
	std::ifstream file;
	/* Catch invalid file path */
	file.open(csvFile_);
	if (!file) {
		std::cerr << "Error opening file '" << csvFile_ << "'.\n";
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
			waypoints_.emplace_back(std::stod(lat), std::stod(lon));
			numWaypoints++;
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
	std::cout << numWaypoints << "/" << lineNo << " waypoints loaded.\n\n";
	return true;
}
