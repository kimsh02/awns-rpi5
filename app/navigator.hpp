#pragma once

#include "concorde.hpp"
#include "gps.hpp"

class Navigator {
    public:
	Navigator(int argc, const char **argv) noexcept;
	void start(void) noexcept;
	void stop(void);

    private:
	const char  *prog_; /* Executable name */
	const int    argc_; /* User arg count */
	const char **argv_; /* User arg vector */

	GPSClient	  gps_;	     /* GPS client */
	ConcordeTSPSolver concorde_; /* Concorde TSP solver */

	// std::pair<double, double> currPos;  /* Current position of system */
	// std::size_t		  nextDest; /* Next waypoint to visit */
	// std::vector<std::pair<double, double> > &tour_; /* Referenced tour from
	// 						   Concorde */

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
	void		      solveTSP(void);
	void		      retryPrompt(const char *) noexcept;
	void		      logFix(const GPSFix &) noexcept;
};
