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

	std::optional<GPSFix> readFix(int timeout_us = 5000000);

    private:
	gps_data_t  data_;
	const char *host_;
	const char *port_;
	bool	    connected_;
};
