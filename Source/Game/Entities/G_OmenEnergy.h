/*! \file G_OmenEnergy.h
	\copyright Copyright (c) 2010 Sunside Inc., All Rights Reserved.
	\copyright See Crow/LICENSE for licensing terms.
*/

#pragma once

#include <Engine/World/Entity.h>
#include <Runtime/PushPack.h>

namespace world {

class G_OmenEnergy : public Entity
{
public:

	G_OmenEnergy();
	virtual ~G_OmenEnergy();

protected:

	virtual void Tick(
		int frame,
		float dt,
		const xtime::TimeSlice &time
	);
};

} // world

E_DECL_SPAWN(RADNULL_API, info_omen_energy)

#include <Runtime/PopPack.h>
