#pragma once

#include <nlohmann/json.hpp>

#include <fstream>

#include "concorde.hpp"
#include "gps.hpp"

using json = nlohmann::json;

class Navigator {
    public:
	Navigator(int argc, const char **argv) noexcept;
	void		    start(void) noexcept;
	void		    setProximityRadius(double) noexcept;
	void		    setSimulationVelocity(double) noexcept;
	std::optional<json> getOutput(void);
	void		    stop(void);

    private:
	static constexpr double earthRadius_{
		6371000.0
	}; /* Earth's radius in meters */
	GPSClient	  gps_;	     /* GPS client */
	ConcordeTSPSolver concorde_; /* Concorde TSP solver */
	const char	 *prog_;     /* Executable name */
	const int	  argc_;     /* User arg count */
	const char	**argv_;     /* User arg vector */
	bool ready_; /* Flag to mark weather navigator is ready to output
		       instructions */
	const std::vector<std::pair<double, double> >
				 &tour_;    /* Referenced tour from Concorde */
	std::pair<double, double> currPos_; /* Current position of system */
	std::size_t nextDest_;		 /* Index of next waypoint to visit */
	std::pair<double, double> dest_; /* Coordinates of next destination */
	bool	    inMotion_; /* Flag to mark whether system is in motion */
	std::size_t bearing_;  /* Direction of movement of system */
	double	    proximityRadius_;	 /* Proximity radius threshold for
					 determining if system has arrived at
					 waypoint */
	double	    simulationVelocity_; /* Velocity of downstream motor
					    controller */
	std::filesystem::path logDir_;	 /* Optional path to directory of log
					  files */
	std::filesystem::path csvFile_;	 /* Path to CSV file for run mode */
	std::ofstream	      logFile_;	 /* Optional log file stream */

	void		  run(void);
	void		  gpspoll(bool);
	[[noreturn]] void solve(void);
	[[noreturn]] void help(void) noexcept;

	std::optional<json>   gpsOutput(void);
	std::optional<json>   simulationVelocityOutput(void);
	std::tm		      localTime(void);
	std::string	      logWithTimestamp(const std::string &);
	void		      logPrint(const std::string &, bool);
	std::string	      logCoordinates(const std::pair<double, double> &);
	void		      setupForNavOutput(void);
	bool		      testGPSConnection(void);
	bool		      readCSV(void);
	bool		      checkValidDir(const std::filesystem::path &);
	std::filesystem::path expandTilde(const std::filesystem::path &);
	bool		      setCSVDir(void);
	bool		      setTSPDir(void);
	bool		      setSolDir(void);
	bool		      setGraphDir(void);
	void		      setLogDir(void);
	bool		      setLogDirHelper(void);
	void		      printPath(const std::filesystem::path &);
	void		      makeSolutions(void);
	void		      solveTSPMeasureTime(void);
	void		      concordeTSP(void);
	void		      setDirectories(bool, bool);
	std::optional<std::pair<double, double> > getDest(void);
	void   retryPrompt(const char *) noexcept;
	void   logFix(const GPSFix &) noexcept;
	bool   waypointReached(const std::pair<double, double> &,
			       const std::pair<double, double> &) const noexcept;
	double calculateBearing(const std::pair<double, double> &,
				const std::pair<double, double> &) noexcept;
	std::pair<double, double>
	computeNewPosition(const std::pair<double, double> &, double) noexcept;

	/* Inline utility methods to convert from degrees to radians and vice
	   versa */
	constexpr double deg2rad(double deg) noexcept
	{
		return deg * M_PI / 180.0;
	}

	constexpr double rad2deg(double rad) noexcept
	{
		return rad * 180.0 / M_PI;
	}
};
