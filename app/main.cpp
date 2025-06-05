#include "navigator.hpp"

int main(int argc, const char **argv)
{
	/* Example usage of API */

	/* Instantiate navigator with user args*/
	Navigator nav{ argc, argv };
	/* Ask navigator to parse args and setup for execution */
	nav.start();

	/* Spit out downstream controller directions */

	/* nav.stop(); */
}
