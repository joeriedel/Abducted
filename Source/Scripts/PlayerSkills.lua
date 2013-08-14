-- PlayerSkills.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerSkills = Class:New()
PlayerSkills.DebugAllAbilitiesEnabled = false
PlayerSkills.UnlimitedSkillPointsCheat = true

PlayerSkills.Skills = {}

PlayerSkills.Skills.ManipulateRegen = {
	Title = "SKILL_MANIPULATE_REGEN_TITLE",
	ShortDescription = "SKILL_MANIPULATE_REGEN_SHORT_DESCRIPTION",
	LongDescription = "SKILL_MANIPULATE_REGEN_LONG_DESCRIPTION",
	Graphics = {
			Icon = {
				Pos={415,8},
				Material="UI/manipulate_default1_M"
			},
			Lines = {
				{
					Material="SkillsCurve1",
					Pos={249, 54} 
				},
				{
					Material="SkillsCurve2",
					Pos={527,54}
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_COOLDOWN").." "..tostring(skill[level].CoolDown)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.ManipulateRegen
	end,
	Upgrade = function(skill)
		PlayerSkills.ManipulateRegen = PlayerSkills.ManipulateRegen + 1
	end,
	[0] = {
		CoolDown = 9,
	},
	{
		Cost = 300,
		CoolDown = 8
	},
	{
		Cost = 500,
		CoolDown = 7
	},
	{
		Cost = 800,
		CoolDown = 6
	},
	{
		Cost = 1200,
		CoolDown = 5
	}
}

PlayerSkills.Skills.ShieldRegen = {
	Title = "SKILL_SHIELD_REGEN_TITLE",
	ShortDescription = "SKILL_SHIELD_REGEN_SHORT_DESCRIPTION",
	LongDescription = "SKILL_SHIELD_REGEN_LONG_DESCRIPTION",
	Requires = {
		{"ManipulateRegen", 1}
	},
	Graphics = {
			Icon = {
				Pos={212,224},
				Material="UI/shield_default1_M"
			},
			Lines = { 
				{
					Material="SkillsCurve2L",
					Pos={46,  262}  
				},
				{
					Material="SkillsCurve3J",
					Pos={249,340}
				}
			}
	},
	Stats = function(skill, level)
		local pct = math.floor(1.25/skill[level].Multiplier*100)
		return StringTable.Get("SKILL_RECHARGE_RATE").." "..tostring(pct).."%"
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.ShieldRegen
	end,
	Upgrade = function(skill)
		PlayerSkills.ShieldRegen = PlayerSkills.ShieldRegen + 1
	end,
	[0] = {
		Multiplier = 1.25
	},
	{
		Multiplier = 1,
		Cost = 300
	},
	{
		Multiplier = 0.666,
		Cost = 500
	},
	{
		Multiplier = 0.5,
		Cost = 800
	},
	{
		Multiplier = 0.333,
		Cost = 1200
	}
}

PlayerSkills.PulseExplodeTime = {8, 10}
PlayerSkills.Skills.PulseRegen = {
	Title = "SKILL_PULSE_REGEN_TITLE",
	ShortDescription = "SKILL_PULSE_REGEN_SHORT_DESCRIPTION",
	LongDescription = "SKILL_PULSE_REGEN_LONG_DESCRIPTION",
	Requires = {
		{"ManipulateRegen", 1}
	},
	Graphics = {
			Icon = {
				Pos={619,224},
				Material="UI/pulse_default1_M"
			},
			Lines = {
				{
					Material="SkillsCurve2L",
					Pos={527,  342},
					Rotation = 180
				},
				{
					Material="SkillsCurve3J",
					Pos={732,262},
					Rotation = 180
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_COOLDOWN").." "..tostring(skill[level].CoolDown)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.PulseRegen
	end,
	Upgrade = function(skill)
		PlayerSkills.PulseRegen = PlayerSkills.PulseRegen + 1
	end,
	[0] = {
		CoolDown = 9
	},
	{
		CoolDown = 8,
		Cost = 300
	},
	{
		CoolDown = 7,
		Cost = 500
	},
	{
		CoolDown = 6,
		Cost = 800
	},
	{
		CoolDown = 5,
		Cost = 1200
	}
}



PlayerSkills.Skills.ManipulateSkill = {
	Title = "SKILL_MANIPULATE_SKILL_TITLE",
	ShortDescription = "SKILL_MANIPULATE_SKILL_SHORT_DESCRIPTION",
	LongDescription = "SKILL_MANIPULATE_SKILL_LONG_DESCRIPTION",
	Requires = {
		{"PulseRegen", 1}, {"ShieldRegen", 1}
	},
	Graphics = {
			Icon = {
				Pos={415,360},
				Material="UI/manipulate_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={460,  480},
					Rotation = 180
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_MANIPULATE_SKILL_LEVEL"..tonumber(level))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.ManipulateSkill
	end,
	Upgrade = function(skill)
		PlayerSkills.ManipulateSkill = PlayerSkills.ManipulateSkill + 1
	end,
	[0] = {
	},
	{
		Cost = 900
	},
	{
		Cost = 1500
	}
}

PlayerSkills.Skills.ShieldDuration = {
	Title = "SKILL_SHIELD_DURATION_TITLE",
	ShortDescription = "SKILL_SHIELD_DURATION_SHORT_DESCRIPTION",
	LongDescription = "SKILL_SHIELD_DURATION_LONG_DESCRIPTION",
	Requires = {
		{"ShieldRegen", 1}
	},
	Graphics = {
			Icon = {
				Pos={5 ,360},
				Material="UI/shield_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={50,  478},
					Rotation = 180
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_DURATION"):format(skill[level].MaxDuration)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.ShieldDuration
	end,
	Upgrade = function(skill)
		HUD:RechargeShield(true) -- instant
		PlayerSkills.ShieldDuration = PlayerSkills.ShieldDuration + 1
	end,
	[0] = {
		MaxDuration = 15
	},
	{
		MaxDuration = 30,
		Cost = 600
	},
	{
		MaxDuration = 45,
		Cost = 900
	},
	{
		MaxDuration = 60,
		Cost = 1200
	}
}

--[[
PlayerSkills.Skills.GrabAllYouCan = {
	Title = "SKILL_GRABALLYOUCAN_TITLE",
	Description = "SKILL_GRABALLYOUCAN_DESCRIPTION",
	Requires = {
		{ "IWillMoveYou", 1 }
	},
	[0] = {
	},
	{
		Cost = 900
	},
	{
		Cost = 1500
	}
}

PlayerSkills.Skills.TheHand = {
	Title = "SKILL_THEHAND_TITLE",
	Description = "Skill_THEHAND_DESCRIPTION",
	Requires = {
		{ "ManipulateRegen", 4 },
		{ "IWillMoveYou", 2 },
		{ "GrabAllYouCan", 2 }
	},
	[0] = {
	},
	{
		Cost = 1500
	},
	{
		Cost = 2100
	},
	{
		Cost = 3000
	}
}

PlayerSkills.Skills.GoLong = {
	Title = "SKILL_GOLONG_TITLE",
	Description = "SKILL_GOLONG_DESCRIPTION",
	[0] = {
		MaxDuration = 15
	},
	{
		MaxDuration = 30,
		Cost = 600
	},
	{
		MaxDuration = 45,
		Cost = 900
	},
	{
		MaxDuration = 60,
		Cost = 1200
	}
}

PlayerSkills.Skills.LetMeEatCake = {
	Title = "SKILL_LETMEEATCAKE_TITLE",
	Description = "SKILL_LETMEEATCAKE_DESCRIPTION",
	[0] = {
	},
	{
		TimeCost = 20,
		Cost = 300
	},
	{
		TimeCost = 15,
		Cost = 500
	},
	{
		TimeCost = 10,
		Cost = 800
	},
	{
		TimeCost = 5,
		Cost = 1200
	}
}

PlayerSkills.Skills.PowerBubble = {
	Title = "SKILL_POWERBUBBLE_TITLE",
	Description = "SKILL_POWERBUBBLE_DESCRIPTION",
	[0] = {
	},
	{
		Cost = 600
	},
	{
		Cost = 900
	},
	{
		Cost = 1200
	}
}

PlayerSkills.Skills.Defender = {
	Title = "SKILL_DEFENDER_TITLE",
	Description = "SKILL_DEFENDER_DESCRIPTION",
	[0] = {
	},
	{
		Cost = 1500
	},
	{
		Cost = 2100
	},
	{
		Cost = 3000
	}
}

PlayerSkills.Skills.BigHit = {
	Title = "SKILL_BIGHIT_TITLE",
	Description = "SKILL_BIGHIT_DESCRIPTION",
	[0] = {
		Multiplier = 1
	},
	{
		Multiplier = 1.25,
		Cost = 600
	},
	{
		Multiplier = 1.5,
		Cost = 900
	}
}

PlayerSkills.Skills.DropYourWeapon = {
	Title = "SKILL_DROPYOURWEAPON_TITLE",
	Description = "SKILL_DROPYOURWEAPON_DESCRIPTION",
	[0] = {
	},
	{
		MaxMines = 1,
		Cost = 600
	},
	{
		MaxMines = 2,
		Cost = 900
	},
	{
		MaxMines = 4,
		Cost = 1200
	}
}

PlayerSkills.Skills.FastestGunInSpace = {
	Title = "SKILL_FASTESTGUNINSPACE_TITLE",
	Description = "SKILL_FASTESTGUNINSPACE_DESCRIPTION",
	[0] = {
	},
	{
		Window = 1,
		Cost = 600,
		DamageMultiplier = 0.5,
		AreaMultiplier = 0.5
	},
	{
		Window = 2,
		Cost = 900,
		DamageMultiplier = 0.5,
		AreaMultiplier = 0.5
	},
	{
		Window = 3,
		Cost = 1200,
		DamageMultiplier = 0.5,
		AreaMultiplier = 0.5
	}
}

PlayerSkills.Skills.PureEnergy = {
	Title = "SKILL_PUREENERGY_TITLE",
	Description = "SKILL_PUREENERGY_DESCRIPTION",
	[0] = {
	},
	{
		CoolDown = 3,
		Cost = 1500
	},
	{
		CoolDown = 3,
		Cost = 2100
	},
	{
		CoolDown = 3,
		Cost = 3000
	}
}

PlayerSkills.Skills.Omega = {
	Title = "SKILL_OMEGA_TITLE",
	Description = "SKILL_OMEGA_DESCRIPTION",
	[0] = {
	},
	{
		Cost = 10000
	}
}

]]

function PlayerSkills.Load(self)
	self.Pulse = 0
	
	self.armUnlocked = Persistence.ReadBool(SaveGame, "armUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	self.manipulateUnlocked = Persistence.ReadBool(SaveGame, "manipulateUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	self.shieldUnlocked = Persistence.ReadBool(SaveGame, "shieldUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	self.pulseUnlocked = Persistence.ReadBool(SaveGame, "pulseUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	
	self.ManipulateSkill = Persistence.ReadNumber(SaveGame, "skills/ManipulateSkill", 0)
	self.ManipulateRegen = Persistence.ReadNumber(SaveGame, "skills/ManipulateRegen", 0)
	
	self.ShieldDuration = Persistence.ReadNumber(SaveGame, "skills/ShieldDuration", 0)
	self.ShieldRegen = Persistence.ReadNumber(SaveGame, "skills/ShieldRegen", 0)
	
	self.PulseRegen = Persistence.ReadNumber(SaveGame, "skills/PulseRegen", 0)
	
	self.SkillPoints = Persistence.ReadNumber(SaveGame, "skillPoints", 0)
	
	if (PlayerSkills.UnlimitedSkillPointsCheat) then
		self.SkillPoints = 999999
	end
end

function PlayerSkills.Save(self)
	
	Persistence.WriteNumber(SaveGame, "skills/ManipulateSkill", self.ManipulateSkill)
	Persistence.WriteNumber(SaveGame, "skills/ManipulateRegen", self.ManipulateRegen)
	
	Persistence.WriteNumber(SaveGame, "skills/ShieldDuration", self.ShieldDuration)
	Persistence.WriteNumber(SaveGame, "skills/ShieldRegen", self.ShieldRegen)
	
	Persistence.WriteNumber(SaveGame, "skills/PulseRegen", self.PulseRegen)
	
	Persistence.WriteNumber(SaveGame, "skillPoints", self.SkillPoints)
	
end

function PlayerSkills.ManipulateSkillLevel(self)
	return self.ManipulateSkill
end

function PlayerSkills.ManipulateRechargeTime(self)
	return PlayerSkills.Skills.ManipulateRegen[self.ManipulateRegen].CoolDown
end

function PlayerSkills.ShieldRechargeTime(self, usedTime)
	return usedTime * PlayerSkills.Skills.ShieldRegen[self.ShieldRegen].Multiplier
end

function PlayerSkills.PulseRechargeTime(self)
	return PlayerSkills.Skills.PulseRegen[self.PulseRegen].CoolDown
end

function PlayerSkills.MaxShieldTime(self)
	return PlayerSkills.Skills.ShieldDuration[self.ShieldDuration].MaxDuration
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
