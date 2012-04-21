// G_Exports.cpp
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#include <Runtime/ReflectMap.h>
#include <Engine/World/Entity.h>
#include "G_ViewController.h"
#include "G_ScreenPawn.h"
#include "G_ScreenProjectile.h"
#include "G_OmenEnergy.h"
#include "G_Thwap.h"

E_EXPORT(RADNULL_API, view_controller)
E_EXPORT(RADNULL_API, info_player_start)
E_EXPORT(RADNULL_API, info_enemy_start)
E_EXPORT(RADNULL_API, info_projectile)
E_EXPORT(RADNULL_API, info_omen_energy)
E_EXPORT(RADNULL_API, x_thwap)
E_EXPORT(RADNULL_API, y_thwap)
E_EXPORT(RADNULL_API, z_thwap)

namespace spawn {

void G_Exports() {}

} // spawn
