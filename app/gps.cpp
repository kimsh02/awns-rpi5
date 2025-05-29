#include "gps.hpp"

#include <gps.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <optional>

/* Constructor */
GPSClient::GPSClient(const char *host, const char *port) noexcept
	: host_(host),
	  port_(port),
	  connected_(false)
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
		/* If GPS reports at least latitude and longitude (and maybe not
		   altitude), return GPSFix struct */
		if (data_.fix.mode >= MODE_2D) {
			return GPSFix{ data_.fix.latitude,
				       data_.fix.longitude };
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
		auto optFix{ readFix() };
		if (optFix) {
			return optFix;
		}
		tries--;
	}
	return std::nullopt;
}
