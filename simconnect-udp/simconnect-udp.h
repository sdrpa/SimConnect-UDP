
#pragma once

/**
 * Required enums, constants and structures required to exchange data with FS through the SimConnect.
*/

// FS simulation phyiscs frames rate per second (currently unused)
//const double SIM_UPDATE_RATE = 30;

enum DATA_DEFINE_ID {
	DEFINITION_AIRCRAFT_POSITION
};

enum REQUEST_ID {
	REQUEST_AIRCRAFT_POSITION
};

/* Start of Structure Definitions */

struct Aircraft {
	double heading; // True aircraft heading in radians
};
