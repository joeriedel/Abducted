// G_ViewController.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel

#include "G_ViewController.h"
#include <Engine/World/World.h>

namespace world {

G_ViewController::G_ViewController() : E_CONSTRUCT_BASE
{
}

G_ViewController::~G_ViewController()
{
}

void G_ViewController::PostSpawn()
{
	target = world->playerPawn;
	E_ViewController::PostSpawn();
}

} // world

namespace spawn {

void *view_controller::Create()
{
	return new (ZWorld) world::G_ViewController();
}

} // spawn
