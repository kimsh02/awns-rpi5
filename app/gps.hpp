#pragma once

#include <optional>
extern "C" {
#include <gps.h>
}

struct GPSFix {
	const double latitude;
	const double longitude;
	const double heading;
};

class GPSClient {
    public:
	/* Default timeout for GPS polling set to 1 second */
	/* Default number of tries to get 2D fix from GPS set to 5 tries */
	GPSClient(const char *host = nullptr, const char *port = nullptr,
		  int timeout_us = 5000000, int max_tries = 5) noexcept;
	~GPSClient(void) noexcept;

	bool connect(void);
	void startStream(void);
	void stopStream(void);

	std::optional<GPSFix> waitReadFix(void);

    private:
	gps_data_t  data_;
	const char *host_;
	const char *port_;
	bool	    connected_;
	const int   timeout_us_;
	const int   max_tries_;
	double	    last_ts_;

	std::optional<GPSFix> readFix(void);
};
