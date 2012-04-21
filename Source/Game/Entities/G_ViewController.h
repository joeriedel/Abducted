// G_ViewController.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel

#include <Engine/World/Entities/E_ViewController.h>
#include <Runtime/PushPack.h>

namespace world {

class G_ViewController : public E_ViewController
{
	E_DECL_BASE(E_ViewController);
public:
	G_ViewController();
	virtual ~G_ViewController();

	virtual void PostSpawn();
};

} // world

E_DECL_SPAWN(RADNULL_API, view_controller)

#include <Runtime/PopPack.h>
