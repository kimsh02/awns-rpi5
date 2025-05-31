#include "navigator.hpp"

int main(int argc, const char **argv)
{
	/* Instantiate navigator and start */
	Navigator nav{ argc, argv };
	nav.start();
}
