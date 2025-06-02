#pragma once

#include <string>
#include <vector>

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

	void retryPrompt(const char *) noexcept;

	void logFix(const GPSFix &) noexcept;
};
