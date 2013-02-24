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

PlayerSkills.DebugAllAbilitiesEnabled = true

function PlayerSkills.Load(self)
	self.Manipulate = 0
	self.Shield = 0
	self.Pulse = 0
	
	self.armUnlocked = Persistence.ReadBool(SaveGame, "armUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	self.manipulateUnlocked = Persistence.ReadBool(SaveGame, "manipulateUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	self.shieldUnlocked = Persistence.ReadBool(SaveGame, "shieldUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	self.pulseUnlocked = Persistence.ReadBool(SaveGame, "pulseUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	
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

function PlayerSkills.ArmUnlocked(self)
	return self.armUnlocked
end

function PlayerSkills.UnlockArm(self)
	self.armUnlocked = true
	Persistence.WriteBool(SaveGame, "armUnlocked", true)
	SaveGame:Save()
end

function PlayerSkills.ManipulateUnlocked(self)
	return self.manipulateUnlocked
end

function PlayerSkills.UnlockManipulate(self)
	self.manipulateUnlocked = true
	Persistence.WriteBool(SaveGame, "manipulateUnlocked", true)
	SaveGame:Save()
end

function PlayerSkills.ShieldUnlocked(self)
	return self.shieldUnlocked
end

function PlayerSkills.UnlockShield(self)
	self.shieldUnlocked = true
	Persistence.WriteBool(SaveGame, "shieldUnlocked", true)
	SaveGame:Save()
end

function PlayerSkills.PulseUnlocked(self)
	return self.pulseUnlocked
end

function PlayerSkills.UnlockPulse(self)
	self.pulseUnlocked = true
	Persistence.WriteBool(SaveGame, "pulseUnlocked", true)
	SaveGame:Save()
end
