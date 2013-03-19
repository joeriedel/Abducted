// G_PlayerPawn.cpp
// Copyright (c) 2012 Sunside Inc., All Rights Reserved
// Author: Joe Riedel

#include "G_PlayerPawn.h"
#include <Engine/World/World.h>

namespace world {

G_PlayerPawn::G_PlayerPawn() : E_CONSTRUCT_BASE {
}

G_PlayerPawn::~G_PlayerPawn() {
}

int G_PlayerPawn::PostSpawn() {
	return E_PlayerPawn::PostSpawn();
}

} // world

namespace spawn {

void *info_player_start::Create() {
	return new (ZWorld) world::G_PlayerPawn();
}

} // spawn
