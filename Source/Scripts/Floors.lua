-- Floors.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Floors = {}

function Floors.SetWaypointTargetnameState(name, _or, _and)
	local l = World.WaypointsForTargetname(name)
	if (l) then
		for k,v in pairs(l) do
			local state = World.WaypointState(v)
			if (_or ~= nil) then
				state = bit.bor(state, _or)
			end
			if (_and ~= nil) then
				state = bit.band(state, _and)
			end
			World.SetWaypointState(v, state)
		end
	end
end

function Floors.SetWaypointUserIdState(name, _or, _and)
	local l = World.WaypointsForUserId(name)
	if (l) then
		for k,v in pairs(l) do
			local state = World.WaypointState(v)
			if (_or ~= nil) then
				state = bit.bor(state, _or)
			end
			if (_and ~= nil) then
				state = bit.band(state, _and)
			end
			World.SetWaypointState(v, state)
		end
	end
end
