// G_PlayerPawn.h
// Copyright (c) 2012 Sunside Inc., All Rights Reserved
// Author: Joe Riedel

#include <Engine/World/Entities/E_PlayerPawn.h>
#include <Runtime/PushPack.h>

namespace world {

class G_PlayerPawn : public E_PlayerPawn {
	E_DECL_BASE(E_PlayerPawn);
public:
	G_PlayerPawn();
	virtual ~G_PlayerPawn();

	virtual void PostSpawn();
};

}

E_DECL_SPAWN(RADNULL_API, info_player_start)

#include <Runtime/PopPack.h>
