#include "gps.hpp"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <optional>
#include <thread>

/* Constructor */
GPSClient::GPSClient(const char *host, const char *port, int timeout_us,
		     int max_tries) noexcept : host_{ host },
					       port_{ port },
					       connected_{ false },
					       timeout_us_{ timeout_us },
					       max_tries_{ max_tries },
					       last_ts_{ 0 }
{
	/* Allocates memory for gps_data_t data_ */
	std::memset(&data_, 0, sizeof(data_));
}

/* Destructor */
GPSClient::~GPSClient(void) noexcept
{
	/* If connected, disable GPS stream and deallocate gps_data_t data_ */
	if (connected_) {
		stopStream();
		gps_close(&data_);
	}
}

/* Connect to port of GPS dongle */
bool GPSClient::connect(void)
{
	/* If gps_open() fails, print to stderr and return false */
	if (gps_open(host_, port_, &data_) != 0) {
		std::cerr << "gps_open failed: " << gps_errstr(errno) << "\n";
		return false;
	}
	/* Else set connected_ to true, and return true */
	connected_ = true;
	return true;
}

/* Start streaming GPS data */
void GPSClient::startStream(void)
{
	/* If GPSClient is connected, start GPS stream */
	if (connected_) {
		gps_stream(&data_, WATCH_ENABLE | WATCH_JSON, nullptr);
	}
}

/* Stop streaming GPS data  */
void GPSClient::stopStream(void)
{
	/* If GPSClient is connected, stop GPS stream */
	if (connected_) {
		gps_stream(&data_, WATCH_DISABLE, nullptr);
	}
}

/* Fix-reading with appropriate error handling */
std::optional<GPSFix> GPSClient::readFix(void)
{
	/* Poll GPS daemon's socket for data */
	if (gps_waiting(&data_, timeout_us_)) {
		/* Read GPS data into data_ struct */
		if (gps_read(&data_, nullptr, 0) < 0) {
			/* If gps_read() returns less than 0, report error and
			   return nullopt */
			std::cerr << "gps_read error: " << gps_errstr(errno)
				  << "\n";
			return std::nullopt;
		}
		/* Check for fresh fix */
		timespec t	= data_.fix.time;
		double	 fix_ts = t.tv_sec + t.tv_nsec * 1e-9;
		/* If stale fix, return nullopt */
		if (fix_ts <= last_ts_) {
			std::cout << fix_ts << " " << last_ts_ << "\n";
			return std::nullopt;
		}
		last_ts_ = fix_ts;
		/* If GPS reports at least latitude and longitude (and maybe not
		   altitude), return GPSFix struct */
		if (data_.fix.mode >= MODE_2D) {
			return GPSFix{ data_.fix.latitude,
				       data_.fix.longitude,
				       data_.fix.track };
		}
	}
	/* Either timeout has expired and no data/not enough data has arrived
	   (for whatever reason), or another error has occured such as the
	   socket closing */
	return std::nullopt;
}

/* Wrapper for readFix(), attemps to get 2D fix reading */
std::optional<GPSFix> GPSClient::waitReadFix(void)
{
	/* If GPSClient is not connected, return nullopt */
	if (!connected_)
		return std::nullopt;

	int tries = max_tries_;
	/* Try to get GPS fix */
	while (tries) {
		/* Try to get 2D fix every 1 seconds */
		std::this_thread::sleep_for(std::chrono::microseconds(1000000));
		auto optFix{ readFix() };
		if (optFix) {
			/* If we get 2D fix, then return fix */
			return optFix;
		}
		tries--;
	}
	/* If we don't get any 2D fix max_tries_, return nullopt */
	return std::nullopt;
}
