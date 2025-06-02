#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

class ConcordeTSPSolver {
    public:
	/* Allow setting via any string-like or path-like type */
	void setCSVFile(auto &&csvFile)
	{
		csvFile_ = std::forward<decltype(csvFile)>(csvFile);
	}

	void setCSVDir(auto &&csvDir)
	{
		csvDir_ = std::forward<decltype(csvDir)>(csvDir);
	}

	/* These have defaults but can be overridden */
	void setTSPDir(auto &&tspDir = "~/.awns-rpi5/tsp")
	{
		tspDir_ = std::forward<decltype(tspDir)>(tspDir);
	}

	void setSolDir(auto &&solDir = "~/.awns-rpi5/sol")
	{
		solDir_ = std::forward<decltype(solDir)>(solDir);
	}

	bool readCSV(void);
	void writeTSPFile(void);
	void solveTSP(void);
	void readTSPSolution(void);
	void plotTSPSolution(void);

    private:
	std::filesystem::path csvFile_; /* Path string to CSV of waypoints*/
	std::filesystem::path csvDir_;	/* Optional path string to directory of
			 CSVs of waypoints */
	std::filesystem::path tspDir_;	/* Path string to directory of TSP
					  files */
	std::filesystem::path solDir_;	/* Path string to directory of solution
					  files */

	std::vector<std::pair<double, double> > waypoints_; /* Lat, lon pairs
							       from CSV */
};
