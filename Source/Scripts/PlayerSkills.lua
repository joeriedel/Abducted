-- PlayerSkills.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerSkills = Class:New()
PlayerSkills.DebugAllAbilitiesEnabled = false
PlayerSkills.UnlimitedSkillPointsCheat = false
PlayerSkills.RefundFraction = 0.75
PlayerSkills.kPulseBaseDamageRadius = 80
PlayerSkills.kPulseBaseDamage = 120
PlayerSkills.kPulseExplodeTime = {10, 12}
PlayerSkills.kArmChatReward = 15
PlayerSkills.kDiscoveryReward = 50
PlayerSkills.kTerminalReward = 150
PlayerSkills.kGlyphReward = 25
PlayerSkills.kAchievementReward = 50

PlayerSkills.Skills = {}

cv_allskills = CVarFunc("allskills", "_cv_allskills")

PlayerSkills.Skills.ManipulateRegen = {
	Title = "SKILL_MANIPULATE_REGEN_TITLE",
	ShortDescription = "SKILL_MANIPULATE_REGEN_SHORT_DESCRIPTION",
	LongDescription = "SKILL_MANIPULATE_REGEN_LONG_DESCRIPTION",
	Graphics = {
			Icon = {
				Pos={415, 8},
				Material="UI/manipulate_default1_M"
			},
			Lines = {
				{
					Material="SkillsCurve1",
					Pos={249, 54} 
				},
				{
					Material="SkillsCurve2",
					Pos={527, 54}
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_COOLDOWN").." "..tostring(skill[level].Cooldown)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.ManipulateRegen
	end,
	Upgrade = function(skill)
		PlayerSkills.ManipulateRegen = PlayerSkills.ManipulateRegen + 1
	end,
	Untrain = function(skill)
		PlayerSkills.ManipulateRegen = 0
	end,
	[0] = {
		Cooldown = 9,
	},
	{
		Cost = 300,
		Cooldown = 8
	},
	{
		Cost = 500,
		Cooldown = 7
	},
	{
		Cost = 800,
		Cooldown = 6
	},
	{
		Cost = 1200,
		Cooldown = 5
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
				Pos={212, 224},
				Material="UI/shield_default1_M"
			},
			Lines = { 
				{
					Material="SkillsCurve2L",
					Pos={46, 262}  
				},
				{
					Material="SkillsCurve3J",
					Pos={249, 340}
				}
			}
	},
	Stats = function(skill, level)
		local pct = math.floor(skill[0].Multiplier/skill[level].Multiplier*100)
		return StringTable.Get("SKILL_RECHARGE_RATE").." "..tostring(pct).."%"
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.ShieldRegen
	end,
	Upgrade = function(skill)
		PlayerSkills.ShieldRegen = PlayerSkills.ShieldRegen + 1
	end,
	Untrain = function(skill)
		PlayerSkills.ShieldRegen = 0
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

PlayerSkills.Skills.PulseRegen = {
	Title = "SKILL_PULSE_REGEN_TITLE",
	ShortDescription = "SKILL_PULSE_REGEN_SHORT_DESCRIPTION",
	LongDescription = "SKILL_PULSE_REGEN_LONG_DESCRIPTION",
	Requires = {
		{"ManipulateRegen", 1}
	},
	Graphics = {
			Icon = {
				Pos={619, 224},
				Material="UI/pulse_default1_M"
			},
			Lines = {
				{
					Material="SkillsCurve2L",
					Pos={527, 342},
					Rotation = 180
				},
				{
					Material="SkillsCurve3J",
					Pos={732, 262},
					Rotation = 180
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_COOLDOWN").." "..tostring(skill[level].Cooldown)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.PulseRegen
	end,
	Upgrade = function(skill)
		PlayerSkills.PulseRegen = PlayerSkills.PulseRegen + 1
	end,
	Untrain = function(skill)
		PlayerSkills.PulseRegen = 0
	end,
	[0] = {
		Cooldown = 9
	},
	{
		Cooldown = 8,
		Cost = 300
	},
	{
		Cooldown = 7,
		Cost = 500
	},
	{
		Cooldown = 6,
		Cost = 800
	},
	{
		Cooldown = 5,
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
				Pos={415, 360},
				Material="UI/manipulate_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={460, 480}
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
	Untrain = function(skill)
		PlayerSkills.ManipulateSkill = 0
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
				Pos={5, 360},
				Material="UI/shield_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={50, 478}
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
	Untrain = function(skill)
		PlayerSkills.ShieldDuration = 0
	end,
	[0] = {
		MaxDuration = 20
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

PlayerSkills.Skills.PulseDamageRadius = {
	Title = "SKILL_PULSE_DAMAGE_RADIUS_TITLE",
	ShortDescription = "SKILL_PULSE_DAMAGE_RADIUS_SHORT_DESCRIPTION",
	LongDescription = "SKILL_PULSE_DAMAGE_RADIUS_LONG_DESCRIPTION",
	Requires = {
		{"PulseRegen", 1}
	},
	Graphics = {
			Icon = { 
				Pos={821, 365},
				Material="UI/pulse_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={867, 485}
				}
			}
	},
	Stats = function(skill, level)
		local pct = math.floor(skill[level].Multiplier/skill[0].Multiplier*100)
		return StringTable.Get("SKILL_DAMAGE_RADIUS").." "..tostring(pct).."%"
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.PulseRadius
	end,
	Upgrade = function(skill)
		PlayerSkills.PulseRadius = PlayerSkills.PulseRadius + 1
	end,
	Untrain = function(skill)
		PlayerSkills.PulseRadius = 0
	end,
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

PlayerSkills.Skills.MultiShield = {
	Title = "SKILL_MULTI_SHIELD_TITLE",
	ShortDescription = "SKILL_MULTI_SHIELD_SHORT_DESCRIPTION",
	LongDescription = "SKILL_MULTI_SHIELD_LONG_DESCRIPTION",
	Requires = {
		{"ShieldDuration", 1}
	},
	Graphics = {
			Icon = { 
				Pos={4,626},
				Material="UI/shield_default1_M"
			},
			Lines = {
				{
					Material="SkillsCurve2L",
					Pos={9, 777},
					Rotation = -90
				},
				{ 
					Material="SkillsHorzLines2",
					Pos={124, 667}
				}
			}
	},
	Stats = function(skill, level)
		if (level < 1) then
			return StringTable.Get("SKILL_MULTI_SHIELD_UNTRAINED")
		elseif (level < 3) then
			return StringTable.Get("SKILL_MULTI_SHIELD_EFFECT1"):format(skill[level].TimeCost)
		end
		return StringTable.Get("SKILL_MULTI_SHIELD_EFFECT2"):format(skill[level].TimeCost)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.MultiShield
	end,
	Upgrade = function(skill)
		HUD:RechargeShield(true) -- instant
		PlayerSkills.MultiShield = PlayerSkills.MultiShield + 1
	end,
	Untrain = function(skill)
		PlayerSkills.MultiShield = 0
	end,
	[0] = {
		TimeCost = 0
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

PlayerSkills.Skills.Mines = {
	Title = "SKILL_MINES_TITLE",
	ShortDescription = "SKILL_MINES_SHORT_DESCRIPTION",
	LongDescription = "SKILL_MINES_LONG_DESCRIPTION",
	Requires = {
		{"PulseDamageRadius", 1}
	},
	Graphics = {
			Icon = { 
				Pos={822,631},
				Material="UI/pulse_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={866, 753}
				},
				{ 
					Material="SkillsHorzLines2",
					Pos={532, 667}
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_MINES_LEVEL"..tostring(level))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.Mines
	end,
	Upgrade = function(skill)
		PlayerSkills.Mines = PlayerSkills.Mines + 1
	end,
	Untrain = function(skill)
		PlayerSkills.Mines = 0
	end,
	[0] = {
		MaxMines = 0
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
		MaxMines = 3,
		Cost = 1200
	}
}

PlayerSkills.Skills.MultiManipulate = {
	Title = "SKILL_MULTI_MANIPULATE_TITLE",
	ShortDescription = "SKILL_MULTI_MANIPULATE_SHORT_DESCRIPTION",
	Requires = {
		{"Mines", 1}, {"ManipulateSkill", 1}, {"MultiShield", 1}
	},
	Graphics = {
			Icon = { 
				Pos={415, 629},
				Material="UI/manipulate_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={460, 751}
				}
			}
	},
	Stats = function(skill, level)
		return StringTable.Get("SKILL_MULTI_MANIPULATE_LEVEL"..tostring(level))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.MultiManipulate
	end,
	Upgrade = function(skill)
		PlayerSkills.MultiManipulate = PlayerSkills.MultiManipulate + 1
	end,
	Untrain = function(skill)
		PlayerSkills.MultiManipulate = 0
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

PlayerSkills.Skills.PowerBubble = {
	Title = "SKILL_POWER_BUBBLE_TITLE",
	ShortDescription = "SKILL_POWER_BUBBLE_SHORT_DESCRIPTION",
	LongDescription = "SKILL_POWER_BUBBLE_LONG_DESCRIPTION",
	Requires = {
		{"MultiShield", 3}
	},
	Graphics = {
			Icon = { 
				Pos={142, 834},
				Material="UI/shield_default1_M"
			},
			Lines = {
				{
					Material="SkillsSquiggle1",
					Pos={106, 951}
				}
			}
	},
	Stats = function(skill, level)
		if (level < 1) then
			return nil
		end
		local pct = math.floor(skill[level].RangeMultiplier/skill[1].RangeMultiplier*100)
		return StringTable.Get("SKILL_POWER_BUBBLE_STATS"):format(tostring(pct), tostring(skill[level].Uses))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.PowerBubble
	end,
	Upgrade = function(skill)
		PlayerSkills.PowerBubble = PlayerSkills.PowerBubble + 1
	end,
	Untrain = function(skill)
		PlayerSkills.PowerBubble = 0
	end,
	{
		RangeMultiplier = 1,
		Uses = 1,
		Cost = 600
	},
	{
		RangeMultiplier = 1.5,
		Uses = 2,
		Cost = 900
	},
	{
		RangeMultiplier = 2,
		Uses = 3,
		Cost = 1200
	}
}

PlayerSkills.Skills.TriggerHappy = {
	Title = "SKILL_TRIGGER_HAPPY_TITLE",
	ShortDescription = "SKILL_TRIGGER_HAPPY_SHORT_DESCRIPTION",
	LongDescription = "SKILL_TRIGGER_HAPPY_LONG_DESCRIPTION",
	Requires = {
		{"Mines", 1}
	},
	Graphics = {
			Icon = { 
				Pos={823, 901},
				Material="UI/pulse_default1_M"
			},
			Lines = {
				{
					Material="SkillsSquiggle2",
					Pos={656, 1020}
				}
			}
	},
	Stats = function(skill, level)
		if (level < 1) then
			return StringTable.Get("SKILL_TRIGGER_HAPPY_UNTRAINED")
		end
		local pct1 = math.floor((1-skill[level].DamageMultiplier/skill[0].DamageMultiplier)*100)
		local pct2 = math.floor((1-skill[level].AreaMultiplier/skill[0].AreaMultiplier)*100)
		return StringTable.Get("SKILL_TRIGGER_HAPPY_STATS"):format(tostring(skill[level].Window), tostring(pct1), tostring(pct2))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.TriggerHappy
	end,
	Upgrade = function(skill)
		PlayerSkills.TriggerHappy = PlayerSkills.TriggerHappy + 1
	end,
	Untrain = function(skill)
		PlayerSkills.TriggerHappy = 0
	end,
	[0] = {
		Window = 0,
		DamageMultiplier = 1,
		AreaMultiplier = 1
	},
	{
		Window = 1,
		Cost = 600,
		DamageMultiplier = 0.25,
		AreaMultiplier = 0.25
	},
	{
		Window = 2,
		Cost = 900,
		DamageMultiplier = 0.30,
		AreaMultiplier = 0.30
	},
	{
		Window = 3,
		Cost = 1200,
		DamageMultiplier = 0.4,
		AreaMultiplier = 0.4
	}
}

PlayerSkills.Skills.TheHand = {
	Title = "SKILL_THE_HAND_TITLE",
	ShortDescription = "SKILL_THE_HAND_SHORT_DESCRIPTION",
	LongDescription = "SKILL_THE_HAND_LONG_DESCRIPTION",
	Requires = {
		{"MultiManipulate", 1}
	},
	Graphics = {
			Icon = { 
				Pos={417, 902},
				Material="UI/manipulate_default1_M"
			},
			Lines = {
				{
					Material="SkillsVertLines2",
					Pos={460, 1023}
				}
			}
	},
	Stats = function(skill, level)
		if (level < 1) then
			StringTable.Get("SKILL_THE_HAND_LEVEL0")
		end
		return StringTable.Get("SKILL_THE_HAND_LEVEL"..tonumber(level)):format(skill[level].Cooldown)
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.TheHand
	end,
	Upgrade = function(skill)
		PlayerSkills.TheHand = PlayerSkills.TheHand + 1
	end,
	Untrain = function(skill)
		PlayerSkills.TheHand = 0
	end,
	[0] = {
	},
	{
		Cooldown = 3,
		Cost = 1500
	},
	{
		Cooldown = 3,
		Cost = 2100
	}
}

PlayerSkills.Skills.Defender = {
	Title = "SKILL_DEFENDER_TITLE",
	ShortDescription = "SKILL_DEFENDER_SHORT_DESCRIPTION",
	LongDescription = "SKILL_DEFENDER_LONG_DESCRIPTION",
	Requires = {
		{"PowerBubble", 1}
	},
	Graphics = {
			Icon = { 
				Pos={64, 1242},
				Material="UI/shield_default1_M"
			}
	},
	Stats = function(skill, level)
		if (level < 1) then
			return StringTable.Get("SKILL_DEFENDER_UNSKILLED")
		end
		return StringTable.Get("SKILL_DEFENDER_STATS"):format(tonumber(skill[level].Cooldown))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.Defender
	end,
	Upgrade = function(skill)
		PlayerSkills.Defender = PlayerSkills.Defender + 1
	end,
	Untrain = function(skill)
		PlayerSkills.Defender = 0
	end,
	[0] = {
	},
	{
		Cooldown = 60,
		Cost = 1500
	},
	{
		Cooldown = 30,
		Cost = 2100
	},
	{
		Cooldown = 15,
		Cost = 3000
	}
}

PlayerSkills.Skills.PureEnergy = {
	Title = "SKILL_PURE_ENERGY_TITLE",
	ShortDescription = "SKILL_PURE_ENERGY_SHORT_DESCRIPTION",
	Requires = {
		{"TriggerHappy", 1}
	},
	Graphics = {
			Icon = { 
				Pos={617, 1185},
				Material="UI/pulse_default1_M"
			}
	},
	Stats = function(skill, level)
		if (level < 1) then
			return nil
		end
		return StringTable.Get("SKILL_PURE_ENERGY_LEVEL"..tostring(level)):format(tonumber(skill[level].Cooldown))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.PureEnergy
	end,
	Upgrade = function(skill)
		PlayerSkills.PureEnergy = PlayerSkills.PureEnergy + 1
	end,
	Untrain = function(skill)
		PlayerSkills.PureEnergy = 0
	end,
	[0] = {
	},
	{
		Cooldown = 3,
		Cost = 1500
	},
	{
		Cooldown = 3,
		Cost = 2100
	},
	{
		Cooldown = 3,
		Cost = 3000
	}
}

PlayerSkills.Skills.Omega = {
	Title = "SKILL_OMEGA_TITLE",
	ShortDescription = "SKILL_OMEGA_SHORT_DESCRIPTION",
	LongDescription = "SKILL_OMEGA_LONG_DESCRIPTION",
	Requires = {
		{"TheHand", 2}
	},
	Graphics = {
			Icon = { 
				Pos={415, 1291},
				Material="UI/manipulate_default1_M"
			}
	},
	Stats = function(skill, level)
		if (level < 1) then
			return nil
		end
		return StringTable.Get("SKILL_OMEGA_STATS"):format(tonumber(skill[level].Cooldown))
	end,
	CurrentLevel = function(skill)
		return PlayerSkills.Omega
	end,
	Upgrade = function(skill)
		PlayerSkills.Omega = PlayerSkills.Omega + 1
	end,
	Untrain = function(skill)
		PlayerSkills.Omega = 0
	end,
	[0] = {
	},
	{
		Cost = 10000,
		Cooldown = 1
	}
}

function _cv_allskills()

	PlayerSkills:GiveAllSkills()
	
end

function PlayerSkills.GiveAllSkills(self, silent)

	self.ManipulateSkill = 2
	self.ManipulateRegen = 4
	self.MultiManipulate = 2
	self.TheHand = 2
	self.ShieldDuration = 3
	self.ShieldRegen = 4
	self.PowerBubble = 3
	self.MultiShield = 4
	self.Defender = 3
	self.PulseRadius = 2
	self.PulseRegen = 4
	self.TriggerHappy = 3
	self.PureEnergy = 3
	self.Mines = 3
	self.Omega = 1
	
	Arm:UpdateSkillsUI()
	
	if (not silent) then
		HUD:Print(nil, "All Skills Enabled", nil, false)
		
		-- calculate cost of all skills
		local allSkillsCost = 0
		
		for k,v in pairs(PlayerSkills.Skills) do
		
			for i=1,9 do
				local skill = v[i]
				if (not skill) then
					break
				end
				
				allSkillsCost = allSkillsCost + skill.Cost
			end
		
		end
		
		COutLine(kC_Debug, "Cost for all skill upgrades: %d skp", allSkillsCost)
	end
	
end

function PlayerSkills.Load(self, fromCheckpoint)
	self.Pulse = 0
	
	if (fromCheckpoint) then
		self.armUnlocked = Persistence.ReadBool(SaveGame, "armUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
		self.manipulateUnlocked = Persistence.ReadBool(SaveGame, "manipulateUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
		self.shieldUnlocked = Persistence.ReadBool(SaveGame, "shieldUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
		self.pulseUnlocked = Persistence.ReadBool(SaveGame, "pulseUnlocked", PlayerSkills.DebugAllAbilitiesEnabled)
	else
	-- reset all abilities
		self.armUnlocked = PlayerSkills.DebugAllAbilitiesEnabled
		self.manipulateUnlocked = PlayerSkills.DebugAllAbilitiesEnabled
		self.shieldUnlocked = PlayerSkills.DebugAllAbilitiesEnabled
		self.pulseUnlocked = PlayerSkills.DebugAllAbilitiesEnabled
		
		Persistence.WriteBool(SaveGame, "armUnlocked", self.armUnlocked)
		Persistence.WriteBool(SaveGame, "manipulateUnlocked", self.manipulateUnlocked)
		Persistence.WriteBool(SaveGame, "shieldUnlocked", self.shieldUnlocked)
		Persistence.WriteBool(SaveGame, "pulseUnlocked", self.pulseUnlocked)
	end
	
	self.ManipulateSkill = Persistence.ReadNumber(SaveGame, "skills/ManipulateSkill", 0)
	self.ManipulateRegen = Persistence.ReadNumber(SaveGame, "skills/ManipulateRegen", 0)
	self.MultiManipulate = Persistence.ReadNumber(SaveGame, "skills/MultiManipulate", 0)
	self.TheHand = Persistence.ReadNumber(SaveGame, "skills/TheHand", 0)
	
	self.ShieldDuration = Persistence.ReadNumber(SaveGame, "skills/ShieldDuration", 0)
	self.ShieldRegen = Persistence.ReadNumber(SaveGame, "skills/ShieldRegen", 0)
	self.PowerBubble = Persistence.ReadNumber(SaveGame, "skills/PowerBubble", 0)
	self.MultiShield = Persistence.ReadNumber(SaveGame, "skills/MultiShield", 0)
	self.Defender = Persistence.ReadNumber(SaveGame, "skills/Defender", 0)
	
	self.PulseRadius = Persistence.ReadNumber(SaveGame, "skills/PulseRadius", 0)
	self.PulseRegen = Persistence.ReadNumber(SaveGame, "skills/PulseRegen", 0)
	self.TriggerHappy = Persistence.ReadNumber(SaveGame, "skills/TriggerHappy", 0)
	self.PureEnergy = Persistence.ReadNumber(SaveGame, "skills/PureEnergy", 0)
		
	self.Mines = Persistence.ReadNumber(SaveGame, "skills/Mines", 0)
	self.Omega = Persistence.ReadNumber(SaveGame, "skills/Omega", 0)
	
	self.SkillPoints = Persistence.ReadNumber(SaveGame, "skillPoints", 0)
	
	if (PlayerSkills.UnlimitedSkillPointsCheat) then
		self.SkillPoints = 999999
	end
end

function PlayerSkills.Save(self)
	
	Persistence.WriteNumber(SaveGame, "skills/ManipulateSkill", self.ManipulateSkill)
	Persistence.WriteNumber(SaveGame, "skills/ManipulateRegen", self.ManipulateRegen)
	Persistence.WriteNumber(SaveGame, "skills/MultiManipulate", self.MultiManipulate)
	Persistence.WriteNumber(SaveGame, "skills/TheHand", self.TheHand)
	
	Persistence.WriteNumber(SaveGame, "skills/ShieldDuration", self.ShieldDuration)
	Persistence.WriteNumber(SaveGame, "skills/ShieldRegen", self.ShieldRegen)
	Persistence.WriteNumber(SaveGame, "skills/PowerBubble", self.PowerBubble)
	Persistence.WriteNumber(SaveGame, "skills/MultiShield", self.MultiShield)
	Persistence.WriteNumber(SaveGame, "skills/Defender", self.Defender)
	
	Persistence.WriteNumber(SaveGame, "skills/PulseRadius", self.PulseRadius)
	Persistence.WriteNumber(SaveGame, "skills/PulseRegen", self.PulseRegen)
	Persistence.WriteNumber(SaveGame, "skills/TriggerHappy", self.TriggerHappy)
	Persistence.WriteNumber(SaveGame, "skills/PureEnergy", self.PureEnergy)
	
	Persistence.WriteNumber(SaveGame, "skills/Mines", self.Mines)
	Persistence.WriteNumber(SaveGame, "skills/Omega", self.Omega)
	
	Persistence.WriteNumber(SaveGame, "skillPoints", self.SkillPoints)
	
end

function PlayerSkills.AwardSkillPoints(self, num)
	if (not PlayerSkills.UnlimitedSkillPointsCheat) then
		self.SkillPoints = self.SkillPoints + num
	end
	EventLog:AddEvent(GameDB:ArmDateString(), "!SKILLPOINTS", tostring(num))
	GameNetwork.LogEvent("SkillPointsAwarded", {num=tostring(num)})
end

function PlayerSkills.ManipulateSkillLevel(self)
	if (self.TheHand >= 2) then
		return 99
	end
	return self.ManipulateSkill
end

function PlayerSkills.ManipulateRechargeTime(self)
	if (self.Omega > 0) then
		return 1
	end
	if (self.TheHand > 0) then
		return 3
	end
	return PlayerSkills.Skills.ManipulateRegen[self.ManipulateRegen].Cooldown
end

function PlayerSkills.NumManipulateActions(self)
	if (self.MultiManipulate == 2) then
		return 2
	elseif (self.MultiManipulate == 1) then
		if (math.random() < 0.5) then
			return 2
		end
	end
	return 1
end

function PlayerSkills.ShieldRechargeTime(self, usedTime)
	if (self.Omega > 0) then
		return 1
	end
	return usedTime * PlayerSkills.Skills.ShieldRegen[self.ShieldRegen].Multiplier
end

function PlayerSkills.PulseRechargeTime(self)
	if (self.Omega > 0) then
		return 1
	end
	if (self.PureEnergy > 0) then
		return PlayerSkills.Skills.PureEnergy[self.PureEnergy].Cooldown
	end
	return PlayerSkills.Skills.PulseRegen[self.PulseRegen].Cooldown
end

function PlayerSkills.RapidFireWindow(self)
	return PlayerSkills.Skills.TriggerHappy[self.TriggerHappy].Window
end

function PlayerSkills.MaxShieldTime(self)
	return PlayerSkills.Skills.ShieldDuration[self.ShieldDuration].MaxDuration
end

function PlayerSkills.ShieldMultiActionTimePenalty(self)
	if (self.Omega > 0) then
		return 0
	end
	return PlayerSkills.Skills.MultiShield[self.MultiShield].TimeCost
end

function PlayerSkills.ShieldAutoActivateCooldown(self)
	if (self.Defender < 1) then
		return 0
	end
	return PlayerSkills.Skills.Defender[self.Defender].Cooldown
end

function PlayerSkills.MaxMines(self)
	return PlayerSkills.Skills.Mines[self.Mines].MaxMines
end

function PlayerSkills.PulseDamageRadius(self)
	local r = PlayerSkills.kPulseBaseDamageRadius * PlayerSkills.Skills.PulseDamageRadius[self.PulseRadius].Multiplier
	if (HUD.pulseMode == "Rapid") then
		r = r * PlayerSkills.Skills.TriggerHappy[self.TriggerHappy].AreaMultiplier
	end
	return r
end

function PlayerSkills.PulseDamage(self)
	local r = PlayerSkills.kPulseBaseDamage
	if (HUD.pulseMode == "Rapid") then
		r = r * PlayerSkills.Skills.TriggerHappy[self.TriggerHappy].DamageMultiplier
	end
	return r
end

function PlayerSkills.PowerBubbleZapRange(self)
	if (self.PowerBubble < 1) then
		return 0
	end
	return PlayerSkills.Skills.PowerBubble[self.PowerBubble].RangeMultiplier * 30
end

function PlayerSkills.PowerBubbleZapCount(self)
	if (self.PowerBubble < 1) then
		return 0
	end
	return PlayerSkills.Skills.PowerBubble[self.PowerBubble].Uses
end

function PlayerSkills.ArmUnlocked(self)
	return self.armUnlocked
end

function PlayerSkills.LockArm(self)
	if (self.armUnlocked or PlayerSkills.DebugAllAbilitiesEnabled) then
		self.armUnlocked = false
		Persistence.WriteBool(SaveGame, "armUnlocked", false)
		HUD:AnimateLock({arm=true})
	end
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
