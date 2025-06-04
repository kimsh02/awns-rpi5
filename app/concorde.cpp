#include "concorde.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

/* Helper method to convert decimal degrees to TSPLIB "GEO" format */
/* (deg*100 + min) */
double ConcordeTSPSolver::decimalDegToTSPLIBGEO(double x) noexcept
{
	int    deg     = static_cast<int>(std::floor(x));
	double minutes = (x - deg) * 60.0;
	return deg * 100.0 + minutes;
}

/* Writes out TSP file from waypoints to be used by Concorde executable*/
void ConcordeTSPSolver::writeTSPFile(void)
{
	/* Create TSP file path string */
	std::string basename{ csvFile_.stem().string() };
	std::string tspPath = tspDir_ / (basename + ".tsp");
	/* Write .tsp file in TSPLIB "GEO" format for Concorde */
	std::ofstream tspOut(tspPath);
	tspOut << "NAME: " << basename << "\n";
	tspOut << "TYPE: TSP\n";
	tspOut << "COMMENT: generated from " << csvFile_.filename().string()
	       << "\n";
	tspOut << "DIMENSION: " << waypoints_.size() << "\n";
	tspOut << "EDGE_WEIGHT_TYPE: GEO\n";
	tspOut << "NODE_COORD_SECTION\n";
	for (size_t i = 0; i < waypoints_.size(); ++i) {
		double xx = decimalDegToTSPLIBGEO(waypoints_[i].first);
		double yy = decimalDegToTSPLIBGEO(waypoints_[i].second);
		tspOut << (i + 1) << " " << xx << " " << yy << "\n";
	}
	tspOut << "EOF\n";
	tspOut.close();
	std::cout << "Wrote TSP file: " << tspPath << "\n";
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
	file.close();
	/* If reader was unable to add any waypoints for whatever reason */
	if (waypoints_.empty()) {
		std::cerr << "Error: Unable to add any waypoints.\n";
		return false;
	}
	/* Else print number of waypoints and return true */
	std::cout << numWaypoints << "/" << lineNo << " waypoints loaded.\n\n";
	return true;
}
