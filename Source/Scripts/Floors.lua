-- Floors.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Floors = {}

function Floors.SetWaypointTargetnameState(name, state)
	local l = World.WaypointsForTargetname(name)
	if (l) then
		for k,v in pairs(l) do
			World.SetWaypointState(v, state)
		end
	end
end

function Floors.SetWaypointUserIdState(name, state)
	local l = World.WaypointsForUserId(name)
	if (l) then
		for k,v in pairs(l) do
			World.SetWaypointState(v, state)
		end
	end
end
