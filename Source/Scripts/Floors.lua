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

function Floors.SaveState(self)
	local x = World.NumFloors()
	local y = Persistence.ReadNumber(SaveGame, "Floors/numFloors", 0)
	
	Persistence.WriteNumber(SaveGame, "Floors/numFloors", x)
	
	y = Max(x, y)
	
	for i = 1,y do
		if (i > x) then
			Persistence.DeleteKey(SaveGame, "Floors/floorName"..i)
			Persistence.DeleteKey(SaveGame, "Floors/floorState"..i)
		else
			local s = World.FloorName(i-1)
			local z = World.FloorState(i-1)
			Persistence.WriteString(SaveGame, "Floors/floorName"..i, s)
			Persistence.WriteNumber(SaveGame, "Floors/floorState"..i, z)
		end
	end
	
	x = World.WaypointIds()
	
	if (x) then
		for k,v in pairs(x) do
			local z = World.WaypointState(v)
			Persistence.WriteNumber(SaveGame, "Floors/waypoint_"..v, z)	
		end
	end
end

function Floors.LoadState(self)

	local x = World.NumFloors()
	
	for i = 1,x do
		local s = Persistence.ReadString(SaveGame, "Floors/floorName"..i)
		if (s) then
			local n = World.FindFloor(s)
			if (n >= 0) then
				local z = Persistence.ReadNumber(SaveGame, "Floors/floorState"..i, 0)
				World.SetFloorState(n, z)
			end
		end
	end

	x = World.WaypointIds()
	
	if (x) then
		for k,v in pairs(x) do
		
			local z = Persistence.ReadNumber(SaveGame, "Floors/waypoint_"..v)
			if (z) then
				World.SetWaypointState(v, z)
			end
		
		end
	end
end