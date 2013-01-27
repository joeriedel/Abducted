-- Entity.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

-- Abducted entity class archetypes:

kEntityClass_Monster = 1
kEntityClass_Player  = 2

Entity = Class:New()

function Entity.Spawn(self)
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
