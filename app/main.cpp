extern "C" {
#include <tsp.h>
#include <util.h>
}

#include "navigator.hpp"

int main()
{
	// The Concorde library is intended for CLI use, but this proves you can include headers
	CCdatagroup dat;
	dat.x = nullptr;
	dat.y = nullptr;

	/* Instantiate and start navigator */
	Navigator nav{};
	nav.start();
}
