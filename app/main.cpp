#include <iostream>

#include "navigator.hpp"

int main(int argc, const char **argv)
{
	/* Example usage of API */

	/* Instantiate navigator with user args */
	Navigator nav{ argc, argv };
	/* Ask navigator to parse args and setup for execution via CLI */
	nav.start();

	/* Set proxmity radius (meters) for determining arrival at each
	   waypoint */
	/* Cannot be set to less 1.0 meters (will result to default of 1) */
	nav.setProximityRadius(10.0);

	/* OPTIONAL: Set simulated downstream motor controller speed
	   (meters/second) */
	/* If set, the navigator will run a simulation of the system's GPS
	   position over time using the specified velocity to calculate motor
	   instructions */
	/* If not set or set to 0 (the default), the navigator will take the GPS
	   readings to calculate motor instructions */
	/* Cannot be set to a negative value (will result to default of 0) */
	nav.setSimulationVelocity(1.0);

	/* Spit out downstream controller output */
	/* Must invoke start and set proximity radius beforehand */
	for (auto output{ nav.getOutput() }; output; output = nav.getOutput()) {
		/* Print JSON to stdout */
		std::cout << (*output).dump(2);
	}

	/* Properly stop navigator before exiting */
	nav.stop();
}
