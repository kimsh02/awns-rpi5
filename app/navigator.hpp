#pragma once

#include <string>
#include <vector>

#include "gps.hpp"

class Navigator {
    public:
	Navigator(int argc, const char **argv) noexcept;
	void args(void) noexcept;

    private:
	const int    argc_;
	const char **argv_;

	std::string	    csv_path_;
	std::vector<GPSFix> waypoints_;

	[[noreturn]] void run(void);
	[[noreturn]] void solve(void);

	void help(void) noexcept;

	void writeTSPFile(void);
	void runConcorde(void);
	void readSolution(void);

	void logFix(GPSFix) noexcept;
	bool readCSV(void);
	void retryPrompt(const char *) noexcept;
};
