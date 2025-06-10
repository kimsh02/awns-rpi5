#include "navigator.hpp"

int main(int argc, const char **argv)
{
	/* Example usage of API */

	/* Instantiate navigator with user args*/
	Navigator nav{ argc, argv };
	/* Ask navigator to parse args and setup for execution via CLI */
	nav.start();
	/* Set proxmity radius (meters) for determining arrival at each
	   waypoint */
	nav.setProximityRadius(10);
	/* Set downstream motor controller speed (meters/second) */
	nav.setControllerVelocity(1);
	/* Spit out downstream controller directions */

	/* Properly stop navigator before exiting */
	nav.stop();
}
