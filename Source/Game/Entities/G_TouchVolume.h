/*! \file G_TouchVolume.h
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup game
*/

#include <Engine/World/Entities/E_TouchVolume.h>
#include <Runtime/PushPack.h>

namespace world {

class G_TouchVolume : public E_TouchVolume {
	E_DECL_BASE(G_TouchVolume);
public:
	G_TouchVolume();
	virtual ~G_TouchVolume();
};

} // world

E_DECL_SPAWN(RADNULL_API, info_kill_volume)

#include <Runtime/PopPack.h>
