#include "navigator.hpp"

int main(int argc, const char **argv)
{
	/* Example usage of API */

	/* Instantiate navigator and start */
	Navigator nav{ argc, argv };
	nav.start();

	/* Spit out downstream controller directions */
}
