/*! \file G_OmenEnergy.cpp
	\copyright Copyright (c) 2010 Sunside Inc., All Rights Reserved.
	\copyright See Crow/LICENSE for licensing terms.
*/

#include "G_OmenEnergy.h"
#include <Engine/World/World.h>

namespace world {

G_OmenEnergy::G_OmenEnergy()
{
}

G_OmenEnergy::~G_OmenEnergy()
{
}

void G_OmenEnergy::Tick(int frame, float dt, const xtime::TimeSlice &time)
{
	Entity::Tick(frame, dt, time);

	BBox bbox(ps->bbox);
	bbox.Translate(ps->worldPos);

	BBox player(world->playerPawn->ps->bbox);
	player.Translate(world->playerPawn->ps->worldPos);

	if (bbox.Touches(player))
	{
		if (PushEntityCall("OnPlayerTouched"))
			world->lua->Call("G_OmenEnergy::Tick", 1, 0, 0);
	}
}

} // world


namespace spawn {

void *info_omen_energy::Create()
{
	return new (ZWorld) world::G_OmenEnergy();
}

} // spawn
