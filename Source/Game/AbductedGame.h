// AbductedGame.h
// Copyright (c) 2012 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Abducted/LICENSE for licensing terms.

#pragma once

#include <Engine/Game/Game.h>
#include <Runtime/PushPack.h>

class AbductedGame : public Game {
public:

	AbductedGame();

	virtual bool LoadEntry();
};

#include <Runtime/PopPack.h>
