-- PlayerSkills.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerSkills = Class:New()
PlayerSkills.DebugAllAbilitiesEnabled = false

PlayerSkills.ManipulateRechargeTimes = { -- 3 levels of manipulate upgrades
	15,
	10,
	5
}

PlayerSkills.MaxShieldTime = 30
PlayerSkills.ShieldRechargeMultipliers = {
	1,
	0.66,
	0.4
}

PlayerSkills.PulseExplodeTime = {8, 10}
PlayerSkills.PulseRechargeTimes = {
	7,
	5,
	3
}

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
	if (not (self.armUnlocked or PlayerSkills.DebugAllAbilitiesEnabled)) then
		self.armUnlocked = true
		Persistence.WriteBool(SaveGame, "armUnlocked", true)
		HUD:AnimateUnlock({arm=true})
	end
end

function PlayerSkills.ManipulateUnlocked(self)
	return self.manipulateUnlocked
end

function PlayerSkills.UnlockManipulate(self)
	if (not (self.manipulateUnlocked or PlayerSkills.DebugAllAbilitiesEnabled)) then
		self.manipulateUnlocked = true
		Persistence.WriteBool(SaveGame, "manipulateUnlocked", true)
		HUD:AnimateUnlock({manipulate=true})
	end
end

function PlayerSkills.ShieldUnlocked(self)
	return self.shieldUnlocked
end

function PlayerSkills.UnlockShield(self)
	if (not (self.shieldUnlocked or PlayerSkills.DebugAllAbilitiesEnabled)) then
		self.shieldUnlocked = true
		Persistence.WriteBool(SaveGame, "shieldUnlocked", true)
		HUD:AnimateUnlock({shield=true})
	end
end

function PlayerSkills.PulseUnlocked(self)
	return self.pulseUnlocked
end

function PlayerSkills.UnlockPulse(self)
	if (not (self.pulseUnlocked or PlayerSkills.DebugAllAbilitiesEnabled)) then
		self.pulseUnlocked = true
		Persistence.WriteBool(SaveGame, "pulseUnlocked", true)
		HUD:AnimateUnlock({pulse=true})
	end
end
