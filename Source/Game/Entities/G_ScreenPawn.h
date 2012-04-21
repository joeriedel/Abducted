// G_ScreenPawn.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#pragma once

#include "G_ScreenView.h"
#include <Runtime/PushPack.h>

namespace world {

class G_ScreenPawn : public G_ScreenView
{
	E_DECL_BASE(G_ScreenView);
public:

	G_ScreenPawn();
	virtual ~G_ScreenPawn();
};

} // world

E_DECL_SPAWN(RADNULL_API, info_player_start)
E_DECL_SPAWN(RADNULL_API, info_enemy_start)

#include <Runtime/PopPack.h>
