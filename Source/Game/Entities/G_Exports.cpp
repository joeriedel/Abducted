// G_Exports.cpp
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#include <Runtime/ReflectMap.h>
#include <Engine/World/Entity.h>
#include "G_ViewController.h"
#include "G_PlayerPawn.h"
#include "G_TouchTrigger.h"
#include "G_ManipulatableObject.h"

E_EXPORT(RADNULL_API, view_controller)
E_EXPORT(RADNULL_API, info_player_start)
E_EXPORT(RADNULL_API, info_touch_trigger)
E_EXPORT(RADNULL_API, info_tentacle)
E_EXPORT(RADNULL_API, info_pylon)
E_EXPORT(RADNULL_API, info_custom_manipulatable)

namespace spawn {

void G_Exports() {}

} // spawn
