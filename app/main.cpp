extern "C" {
#include <gps.h>
}

#include "navigator.hpp"

int main()
{
	/* Instantiate and run navigator */
	Navigator nav{};
	nav.run();
}
