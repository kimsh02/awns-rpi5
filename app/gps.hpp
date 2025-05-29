#pragma once

#include <optional>
extern "C" {
#include <gps.h>
}

struct GPSFix {
	const double latitude;
	const double longitude;
};

class GPSClient {
    public:
	GPSClient(const char *host = nullptr,
		  const char *port = nullptr) noexcept;
	~GPSClient(void) noexcept;

	bool connect(void);
	void startStream(void);
	void stopStream(void);

	std::optional<GPSFix> waitReadFix(void);

    private:
	/* Set timeout for GPS polling to 1.2 seconds */
	static constexpr int timeout_us_ = 1200000;
	/* Number of tries to get 2D fix reading from GPS */
	static constexpr int max_tries_ = 5;

	gps_data_t  data_;
	const char *host_;
	const char *port_;
	bool	    connected_;

	std::optional<GPSFix> readFix(void);
};
