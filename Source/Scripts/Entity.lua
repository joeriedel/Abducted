-- Entity.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

-- Abducted entity class archetypes:

kEntityClass_Monster = 1
kEntityClass_Player  = 2

Entity = Class:New()

function Entity.Spawn(self)

	local angle = NumberForString(self.keys.angle, 0)
	
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	self:SetAngles(angleVertex)

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

function Entity.GetSpawnFloorPosition(self)

	if (self.keys.floorNum == nil) then
		return nil
	end
	
	local pos = Vec3ForString(self.keys.origin)
	local floorNum = NumberForString(self.keys.floorNum)
	local triNum = NumberForString(self.keys.floorTriNum)
	
	return World.CreateFloorPosition(pos, floorNum, triNum)

end