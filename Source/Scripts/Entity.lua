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
	end
	
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
		{a[1], a[2], a[3] - 32},
		{a[1], a[2], a[3] + 32}
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