#pragma once

#include <functional>
#include <vector>

#include "concorde.hpp"
#include "gps.hpp"

class Navigator {
    public:
	Navigator(int argc, const char **argv) noexcept;
	void start(void) noexcept;

    private:
	const char  *prog_; /* Executable name */
	const int    argc_; /* User arg count */
	const char **argv_; /* User arg vector */

	std::vector<GPSFix> waypoints_; /* Vector of waypoints */

	[[noreturn]] void run(void);
	[[noreturn]] void solve(void);
	[[noreturn]] void help(void) noexcept;

	bool testGPSConnection(GPSClient &);
	bool readCSV(ConcordeTSPSolver &);

	bool		      checkValidDir(std::filesystem::path &);
	std::filesystem::path expandTilde(const std::filesystem::path &);
	bool		      setTSPDir(ConcordeTSPSolver &);
	bool		      setSolDir(ConcordeTSPSolver &);
	void		      printPath(const std::filesystem::path &);

	void retryPrompt(const char *) noexcept;
	void logFix(const GPSFix &) noexcept;
};
