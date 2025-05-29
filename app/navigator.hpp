#pragma once

#include <string>
#include <vector>

#include "gps.hpp"

class Navigator {
    public:
	Navigator(void) noexcept;
	~Navigator(void) noexcept;

	void run(void);

    private:
	double		    velocity_;
	double		    course_;
	std::string	    csv_path_;
	std::vector<GPSFix> waypoints_;

	void logWaypoint(GPSFix) noexcept;
	bool readCSV(void);
};
