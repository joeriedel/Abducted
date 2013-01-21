/*! \file G_TouchTrigger.cpp
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup game
*/

#include "G_TouchTrigger.h"

namespace world {

G_TouchTrigger::G_TouchTrigger() : E_CONSTRUCT_BASE {
}

G_TouchTrigger::~G_TouchTrigger() {
}

} // world

namespace spawn {

void *info_touch_trigger::Create() {
	return new (ZWorld) world::G_TouchTrigger();
}

} // spawn
