-- Entity.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

-- Abducted entity class archetypes:

kEntityClass_Monster = 1
kEntityClass_Player  = 2
kEntityClass_Object  = 4

Entity = Class:New()

function Entity.Spawn(self)

	if (self.keys.angle) then
		local angle = NumberForString(self.keys.angle, 0)
		local angleVertex = self:Angles()
		angleVertex.pos = {0, 0, angle}
		self:SetAngles(angleVertex)
		self:SetTargetAngles(angleVertex.pos)
	end
	
end

function Entity.SaveFloorPos(self, state)
	local fp = self:FloorPosition()
	if (fp.floor == -1) then
		local classname = StringForString(self.keys.classname, "<null>")
		local targetname = StringForString(self.keys.targetname, "<notarget>")
		error(string.format("Entity.SaveFloorPos: Checkpoint error (%s, %s) is not on a floor!", classname, targetname))
	end
	
	state.floorpos = string.format("%d %d %d", fp.pos[1], fp.pos[2], fp.pos[3])
	state.floorname = World.FloorName(fp.floor)
	
	-- check if we are really over the floor:
	local floorNum = fp.floor
	local floorState = World.FloorState(floorNum)
	World.SetFloorState(floorNum, kFloorState_Enabled)
	
	local pos = Vec3ForString(state.floorpos)
	fp  = World.ClipToFloor(
		{pos[1], pos[2], pos[3] + 32},
		{pos[1], pos[2], pos[3] - 32}
	)
	
	World.SetFloorState(floorNum, floorState)
	
	if (fp == nil) then
		local classname = StringForString(self.keys.classname, "<null>")
		local targetname = StringForString(self.keys.targetname, "<notarget>")
		error(string.format("Entity.SaveFloorPos: Checkpoint error (%s, %s) is not on a floor!", classname, targetname))
	end
end

function Entity.LoadFloorPos(self, state)

	local floorNum = World.FindFloor(state.floorname)
	assert(floorNum ~= -1)
	
	local floorState = World.FloorState(floorNum)
	World.SetFloorState(floorNum, kFloorState_Enabled)
	
	local pos = Vec3ForString(state.floorpos)
	local fp  = World.ClipToFloor(
		{pos[1], pos[2], pos[3] + 16},
		{pos[1], pos[2], pos[3] - 16}
	)
	
	assert(fp)
	self:SetFloorPosition(fp)
	self:Link()
	
	World.SetFloorState(floorNum, floorState)
end

function Entity.Precache(self, path, async)

	self.precached = self.precached or {}
	table.insert(self.precached, World.Precache(path, async))

end

function Entity.TraceMove(self, distance, zangle, currentFloorOnly, stepSize)

	local fwd = nil

	if (type(zangle) == "table") then
		fwd = zangle
	else
		fwd = RotateVecZ({1,0,0}, zangle)
	end
		
	local curFP = nil
	
	if (currentFloorOnly) then
		curFP = self:FloorPosition()
		if (curFP.floor == -1) then
			curFP = nil
		end
	end
	
	local fp = Entity.TraceFloorDir(self, distance, fwd, curFP)
	
	if (fp) then
		return fp, distance
	end
	
	-- slow path
	if (stepSize == nil) then
		stepSize = 10
	end
	
	local d = distance - stepSize
	while (true) do
		if (d < stepSize) then
			break
		end
		
		fp = Entity.TraceFloorDir(self, d, fwd, curFP)
		if (fp) then
			return fp, d
		end
		
		d = d - stepSize
	end
	
	return nil
end

function Entity.TraceFloorDir(self, distance, dir, curFP)

	local testPos = VecAdd(self:WorldPos(), VecScale(dir, distance))

	if (curFP) then
		fp = World.ClipToFloor(
			curFP.floor, 
			{testPos[1], testPos[2], testPos[3]+32},
			{testPos[1], testPos[2], testPos[3]-512}
		)
	else
		fp = World.ClipToFloor(
			{testPos[1], testPos[2], testPos[3]+32},
			{testPos[1], testPos[2], testPos[3]-512}
		)
	end
	
	return fp
end

function Entity.SetRotation(self, angles)
	local angleVertex = self:Angles()
	angleVertex.pos = WrapAngles(angles)
	self:SetAngles(angleVertex)
	self:SetTargetAngles(angles)
end

function Entity.EnableFlags(self, flags, enable)
	local curFlags = self:Flags()
	if (enable) then
		curFlags = bit.bor(curFlags, flags)
	else
		curFlags = bit.band(curFlags, bit.bnot(flags))
	end
	self:SetFlags(curFlags)
end


function Entity.SetFacing(self, zAngle)
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0,  WrapAngle(zAngle)}
	self:SetAngles(angleVertex)
	self:SetTargetAngles(angleVertex.pos)
end

function Entity.SpawnFloorPosition(self)
	local fp = self:LoadFloorPosition()
	if (fp) then
		self.validFloorPosition = true
		self:SetFloorPosition(fp)
	end
	
	return fp
end

function Entity.FindFloor(self, pos)

	local a = pos or self:WorldPos()
	return World.ClipToFloor(
		{a[1], a[2], a[3] + 32},
		{a[1], a[2], a[3] - 32}
	)
	
end

function Entity.LoadFloorPosition(self)

	if (self.keys.waypoint) then
		local waypoints = World.WaypointsForUserId(self.keys.waypoint)
		if (waypoints == nil) then
			error("'"..self.keys.classname.."' unable to find starting waypoint named "..self.keys.waypoint)
		end
		return World.WaypointFloorPosition(waypoints[1])
	end

	if (self.keys.floorNum == nil) then
		return nil
	end
	
	local pos = Vec3ForString(self.keys.origin)
	local floorNum = NumberForString(self.keys.floorNum)
	local triNum = NumberForString(self.keys.floorTri)
	
	return World.CreateFloorPosition(pos, floorNum, triNum)

end