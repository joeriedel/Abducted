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

PlayerSkills.MaxShieldTime = 1000
PlayerSkills.ShieldRechargeMultipliers = {
	0.001,
	0.66,
	0.5
}

PlayerSkills.PulseExplodeTime = {8, 10}
PlayerSkills.PulseRechargeTimes = {
	1,
	20,
	15
}

function PlayerSkills.Load(self)
	self.Manipulate = 0
	self.Shield = 0
	self.Pulse = 0
end

function PlayerSkills.ManipulateRechargeTime(self)
	return PlayerSkills.ManipulateRechargeTimes[self.Manipulate+1]
end

function PlayerSkills.ShieldRechargeTime(self, usedTime)
	return usedTime * PlayerSkills.ShieldRechargeMultipliers[self.Shield+1]
end

function PlayerSkills.PulseRechargeTime(self)
	return PlayerSkills.PulseRechargeTimes[self.Pulse+1]
end
