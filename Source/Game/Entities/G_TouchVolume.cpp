/*! \file G_TouchVolume.cpp
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup game
*/

#include "G_TouchVolume.h"

namespace world {

G_TouchVolume::G_TouchVolume() : E_CONSTRUCT_BASE {
}

G_TouchVolume::~G_TouchVolume() {
}

} // world

namespace spawn {

void *info_kill_volume::Create() {
	return new (ZWorld) world::G_TouchVolume();
}

} // spawn
