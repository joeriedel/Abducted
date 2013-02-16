-- Floors.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Floors = Class:New()

function Floors.SetWaypointTargetnameState(self, name, _or, _and)
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

function Floors.SetWaypointUserIdState(self, name, _or, _and)
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

function Floors.SetFloorState(self, name, _or, _and)

	local floorNum = name
	if (type(name) == "string") then
		floorNum = World.FindFloor(name)
		if (floorNum < 0) then
			return
		end
	end
	
	local state = World.FloorState(floorNum)
	if (_or ~= nil) then
		state = bit.bor(state, _or)
	end
	if (_and ~= nil) then
		state = bit.band(state, _and)
	end
	
	World.SetFloorState(floorNum, state)

end