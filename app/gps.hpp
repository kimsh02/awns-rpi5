#pragma once

#include <optional>
extern "C" {
#include <gps.h>
}

struct GPSFix {
	const double latitude;	/* GPS latitude */
	const double longitude; /* GPS longitude */
	const double heading;	/* GPS bearing from true North */
};

class GPSClient {
    public:
	/* Default timeout for GPS polling set to 1.5 seconds */
	/* Default number of tries to get 2D fix from GPS set to 5 tries */
	GPSClient(const char *host = nullptr, const char *port = nullptr,
		  int timeout_us = 1500000, int max_tries = 5) noexcept;
	~GPSClient(void);

	bool connect(void);
	void startStream(void);
	void stopStream(void);

	std::optional<GPSFix> waitReadFix(void);

    private:
	gps_data_t  data_;	 /* GPS data struct */
	const char *host_;	 /* Host string */
	const char *port_;	 /* Port string */
	bool	    connected_;	 /* GPS connection status */
	const int   timeout_us_; /* GPS connection timeout limit in
				  microseconds */
	const int   max_tries_;	 /* Max attempts to poll GPS */
	double	    last_ts_;	 /* Last GPS poll timestamp to retrieve fresh
				 data */

	std::optional<GPSFix> readFix(void);
};
