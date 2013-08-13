-- PlayerSkills.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerSkills = Class:New()
PlayerSkills.DebugAllAbilitiesEnabled = false
PlayerSkills.UnlimitedSkillPointsCheat = true

PlayerSkills.Data = {}

PlayerSkills.Data.FastHands = {
	Title = "SKILL_FASTHANDS_TITLE",
	ShortDescription = "SKILL_FASTHANDS_SHORT_DESCRIPTION",
	LongDescription = "SKILL_FASTHANDS_LONG_DESCRIPTION",
	Graphics = {
			Icon = {
				Material="UI/manipulate_default1_M",
				Pos={416,9}
			},
			Lines = {
				{
					Material="SkillsCurve1",
					Pos={249, 54} 
				},
				{
					Material="SkillsCurve2",
					Rotation=90,
					Pos={527,54}
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_COOLDOWN").." "..tostring(skill[level].CoolDown)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.FastHands
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

PlayerSkills.Data.IWillMoveYou = {
	Title = "SKILL_IWILLMOVEYOU_TITLE",
	Description = "SKILL_IWILLMOVEYOU_DESCRIPTION",
	Requires = {
		{ "FastHands", 1 }
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

PlayerSkills.Data.GrabAllYouCan = {
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

PlayerSkills.Data.TheHand = {
	Title = "SKILL_THEHAND_TITLE",
	Description = "Skill_THEHAND_DESCRIPTION",
	Requires = {
		{ "FastHands", 4 },
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

PlayerSkills.Data.GimmeBack = {
	Title = "SKILL_GIMMEBACK_TITLE",
	Description = "SKILL_GIMMEBACK_DESCRIPTION",
	[0] = {
		Multiplier = 0.8
	},
	{
		Multiplier = 1,
		Cost = 300
	},
	{
		Multiplier = 1.5,
		Cost = 500
	},
	{
		Multiplier = 2,
		Cost = 800
	},
	{
		Multiplier = 3,
		Cost = 1200
	}
}

PlayerSkills.Data.GoLong = {
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

PlayerSkills.Data.LetMeEatCake = {
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

PlayerSkills.Data.PowerBubble = {
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

PlayerSkills.Data.Defender = {
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

PlayerSkills.Data.QuickDraw = {
	Title = "SKILL_QUICKDRAW_TITLE",
	Description = "SKILL_QUICKDRAW_DESCRIPTION",
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

PlayerSkills.Data.BigHit = {
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

PlayerSkills.Data.DropYourWeapon = {
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

PlayerSkills.Data.FastestGunInSpace = {
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

PlayerSkills.Data.PureEnergy = {
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

PlayerSkills.Data.Omega = {
	Title = "SKILL_OMEGA_TITLE",
	Description = "SKILL_OMEGA_DESCRIPTION",
	[0] = {
	},
	{
		Cost = 10000
	}
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
	
	self.FastHands = 0
	self.SkillPoints = Persistence.ReadNumber(SaveGame, "skillPoints", 0)
	
	if (PlayerSkills.UnlimitedSkillPointsCheat) then
		self.SkillPoints = 999999
	end
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
