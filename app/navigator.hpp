#pragma once

#include <string>
#include <vector>

#include "gps.hpp"

class Navigator {
    public:
	[[noreturn]] void run(void);

    private:
	// double		    velocity_;
	// double		    course_;
	std::string	    csv_path_;
	std::vector<GPSFix> waypoints_;

	void logFix(GPSFix) noexcept;
	bool readCSV(void);
	void bestPath(void) noexcept;
	void retryPrompt(const char *) noexcept;
};
