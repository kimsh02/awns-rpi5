#pragma once

#include <filesystem>
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

	void setTSPDir(auto &&tspDir)
	{
		tspDir_ = std::forward<decltype(tspDir)>(tspDir);
	}

	void setSolDir(auto &&solDir)
	{
		solDir_ = std::forward<decltype(solDir)>(solDir);
	}

	void setGraphDir(auto &&graphDir)
	{
		graphDir_ = std::forward<decltype(graphDir)>(graphDir);
	}

	const std::filesystem::path		      &getCSVDir(void) noexcept;
	const std::vector<std::pair<double, double> > &getTour(void) noexcept;

	bool readCSV(void);
	void writeTSPFile(void);
	void solveTSP(void);
	void readTSPSolution(void);
	void plotTSPSolution(void);

    private:
	std::filesystem::path csvFile_;	  /* Path string to CSV of waypoints*/
	std::filesystem::path tspFile_;	  /* Path string to concorde TSP file */
	std::filesystem::path solFile_;	  /* Path string to concorde solution
					   file */
	std::filesystem::path graphFile_; /* Path string to graph of tour */
	std::filesystem::path csvDir_;	  /* Path string to directory of CSV
					   files */
	std::filesystem::path tspDir_;	  /* Path string to directory of TSP
					  files */
	std::filesystem::path solDir_;	 /* Path string to directory of solution
					  files */
	std::filesystem::path graphDir_; /* Path string to directory of graph
					    files */

	std::vector<std::pair<double, double> > waypoints_; /* Lat, lon pairs
							       from CSV */
	std::vector<int> tourOrder_; /* Order that waypoints should be
					visited */
	std::vector<std::pair<double, double> > tour_; /* Lat, lon pairs of
							       tour */

	double decimalDegToTSPLIBGEO(double) noexcept;
};
