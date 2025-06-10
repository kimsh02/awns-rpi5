#pragma once

#include <nlohmann/json.hpp>

#include "concorde.hpp"
#include "gps.hpp"

using json = nlohmann::json;

class Navigator {
    public:
	Navigator(int argc, const char **argv) noexcept;
	void		    start(void) noexcept;
	void		    setProximityRadius(double) noexcept;
	void		    setControllerVelocity(double) noexcept;
	std::optional<json> getOutput(void);
	std::optional<json> getOutput(double);
	void		    stop(void);

    private:
	GPSClient	  gps_;	     /* GPS client */
	ConcordeTSPSolver concorde_; /* Concorde TSP solver */
	const char	 *prog_;     /* Executable name */
	const int	  argc_;     /* User arg count */
	const char	**argv_;     /* User arg vector */
	bool ready_; /* Flag to mark weather navigator is ready to output
		       instructions */
	const std::vector<std::pair<double, double> >
		&tour_; /* Referenced tour from Concorde */

	// std::pair<double, double> currPos;  /* Current position of system */
	std::size_t nextDest_; /* Index of next waypoint to visit */
	bool	    inMotion_; /* Flag to mark whether system is in motion */
	std::size_t bearing_;  /* Direction of movement of system */
	double	    proximityRadius_;	 /* Proximity radius threshold for
					 determining if system has arrived at
					 waypoint */
	double	    controllerVelocity_; /* Velocity of downstream motor
					    controller */

	void		  run(void);
	void		  gpspoll(bool);
	[[noreturn]] void solve(void);
	[[noreturn]] void help(void) noexcept;

	bool		      testGPSConnection(void);
	bool		      readCSV(void);
	bool		      checkValidDir(std::filesystem::path &);
	std::filesystem::path expandTilde(const std::filesystem::path &);
	bool		      setCSVDir(void);
	bool		      setTSPDir(void);
	bool		      setSolDir(void);
	bool		      setGraphDir(void);
	void		      printPath(const std::filesystem::path &);
	void		      makeSolutions(void);
	void		      solveTSPMeasureTime(void);
	void		      concordeTSP(void);
	void		      setDirectories(bool);
	std::size_t	      nextDest(void);
	void		      retryPrompt(const char *) noexcept;
	void		      logFix(const GPSFix &) noexcept;
};
