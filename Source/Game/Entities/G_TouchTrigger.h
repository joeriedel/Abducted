/*! \file G_TouchTrigger.h
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup game
*/

#include <Engine/World/Entities/E_TouchTrigger.h>
#include <Runtime/PushPack.h>

namespace world {

class G_TouchTrigger : public E_TouchTrigger {
	E_DECL_BASE(G_TouchTrigger);
public:
	G_TouchTrigger();
	virtual ~G_TouchTrigger();
};

} // world

E_DECL_SPAWN(RADNULL_API, info_touch_trigger)

#include <Runtime/PopPack.h>
