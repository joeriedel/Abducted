-- PlayerSkills.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerSkills = Class:New()

PlayerSkills.ManipulateRechargeTimes = { -- 3 levels of manipulate upgrades
	15,
	10,
	5
}

function PlayerSkills.Load(self)
	self.Manipulate = 0
end

function PlayerSkills.ManipulateRechargeTime(self)
	return PlayerSkills.ManipulateRechargeTimes[self.Manipulate+1]
end
