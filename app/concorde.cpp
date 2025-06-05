#include "concorde.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

/* Helper method to convert decimal degrees to TSPLIB "GEO" format */
/* (deg*100 + min) */
double ConcordeTSPSolver::decimalDegToTSPLIBGEO(double x) noexcept
{
	// 1) Remember the sign (+1 or -1)
	double sign = (x < 0.0) ? -1.0 : 1.0;
	// 2) Work with the absolute value
	double abs_val = std::abs(x);
	// 3) Integer degrees from the absolute value
	int deg = static_cast<int>(std::floor(abs_val));
	// 4) Fractional part → minutes
	double minutes = (abs_val - deg) * 60.0;
	// 5) Combine as (degrees * 100 + minutes), then reapply sign
	return sign * (deg * 100.0 + minutes);
}

/* Writes out TSP file from waypoints to be used by Concorde executable*/
void ConcordeTSPSolver::writeTSPFile(void)
{
	/* Create TSP file path string */
	std::string basename{ csvFile_.stem().string() };
	tspFile_ = tspDir_ / (basename + ".tsp");
	/* Set tspFile_ */

	/* Write .tsp file in TSPLIB "GEO" format for Concorde */
	std::ofstream tspOut(tspFile_);
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
	std::cout << "Wrote TSP file: " << tspFile_ << ".\n";
}

/* Calls on Concorde to solve the TSP file and write out solution file */
void ConcordeTSPSolver::solveTSP(void)
{
	/* TODO: Record time performance */

	/* Create solution file path string */
	std::string basename{ tspFile_.stem().string() };
	solFile_ = solDir_ / (basename + ".sol");
	/* Run Concorde executable to get optimal solution */
}

/* Getter for CSV directory */
const std::filesystem::path &ConcordeTSPSolver::getCSVDir(void) noexcept
{
	return csvDir_;
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
	std::cout << numWaypoints << "/" << lineNo << " waypoints loaded for "
		  << csvFile_ << ".\n\n";
	return true;
}
