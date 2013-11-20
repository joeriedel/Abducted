-- Achievements.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Achievements = {
	Ids = {
		Telekinesis = {
			Id = "ID_Telekinesis",
			Icon = "UI/acv_telekinesis_M",
			String = "ACV_TELEKINESIS"
		},
		Disintegrate = {
			Id = "ID_Disintegrate",
			Icon = "UI/acv_disintegrate_M",
			String = "ACV_GUTS"
		},
		Guts = {
			Id = "ID_Guts",
			Icon = "UI/acv_guts_M",
			String = "ACV_GUTS"
		},
		ItBegins = {
			Id = "ID_ItBegins",
			Icon = "UI/acv_itbegins_M",
			String = "ACV_ITBEGINS"
		},
		Interface = {
			Id = "ID_Interface",
			Icon = "UI/acv_interface_M",
			String = "ACV_INTERFACE"
		},
		Fearless = {
			Id = "ID_Fearless",
			Icon = "UI/acv_fearless_M",
			String = "ACV_FEARLESS"
		},
		Einstein = {
			Id = "ID_Einstein",
			Icon = "UI/acv_einstein_M",
			String = "ACV_EINSTEIN"
		},
		Brawn = {
			Id = "ID_Brawn",
			Icon = "UI/acv_brawn_M",
			String = "ACV_BRAWN"
		},
		Database = {
			Id = "ID_Database",
			Icon = "UI/acv_database_M",
			String = "ACV_DATABASE"
		},
		Codex = {
			Id = "ID_Codex",
			Icon = "UI/acv_codex_M",
			String = "ACV_CODEX"
		},
		Vertical = {
			Id = "ID_Vertical",
			Icon = "UI/acv_vertical_M",
			String = "ACV_VERTICAL"
		},
		Symbolism = {
			Id = "ID_Symbolism",
			Icon = "UI/acv_symbolism_M",
			String = "ACV_SYMBOLISM"
		},
		Curious = {
			Id = "ID_Curious",
			Icon = "UI/acv_curious_M",
			String = "ACV_CURIOUS"
		},
		Revelation = {
			Id = "ID_Revelation",
			Icon = "UI/acv_revelation_M",
			String = "ACV_REVELATION"
		},
		TheHunt = {
			Id = "ID_TheHunt",
			Icon = "UI/acv_thehunt_M",
			String = "ACV_THEHUNT"
		},
		TheShip = {
			Id = "ID_TheShip",
			Icon = "UI/acv_theship_M",
			String = "ACV_THESHIP"
		}
	}
}

function Achievements.Init(self)

	for k,v in pairs(Achievements.Ids) do
		v.Icon = World.Load(v.Icon)
	end

	self.Sound = World.LoadSound("Audio/achievement")
end

function Achievements.Load(self)
	self.numShotWithPulse = Persistence.ReadNumber(SaveGame, "acv_shot_with_pulse", 0)
	self.numSquishedBugs = Persistence.ReadNumber(SaveGame, "acv_squished_bugs", 0)
	self.numTopics = Persistence.ReadNumber(SaveGame, "acv_topics", 0)
	self.numDiscoveries = Persistence.ReadNumber(SaveGame, "acv_discoveries", 0)
	self.numManipulated = Persistence.ReadNumber(SaveGame, "acv_manipulated", 0)
	self.numGlyphs = Persistence.ReadNumber(SaveGame, "acv_glyphs", 0)
end

function Achievements.ShotWithPulse(self)
	self.numShotWithPulse = self.numShotWithPulse + 1
	Persistence.WriteNumber(SaveGame, "acv_shot_with_pulse", self.numShotWithPulse)
	if (self.numShotWithPulse >= 10) then	
		Achievements:Award("Disintegrate")
	end
	GameNetwork.LogEvent("ShotWithPulse")
end

function Achievements.SquishedBugs(self, num)
	self.numSquishedBugs = self.numSquishedBugs + num
	Persistence.WriteNumber(SaveGame, "acv_squished_bugs", self.numSquishedBugs)
	if (self.numSquishedBugs >= 5) then	
		Achievements:Award("Guts")
	end
	for i=1,num do
		GameNetwork.LogEvent("SquishedBug")
	end
end

function Achievements.DiscussedTopic(self)
	self.numTopics = self.numTopics + 1
	Persistence.WriteNumber(SaveGame, "acv_topics", self.numTopics)
	if (self.numTopics == 50) then	
		Achievements:Award("Curious")
	end
	GameNetwork.LogEvent("DiscussedTopic")
end

function Achievements.Discovered(self, id)
	self.numDiscoveries = self.numDiscoveries + 1
	Persistence.WriteNumber(SaveGame, "acv_discoveries", self.numDiscoveries)
	if (self.numDiscoveries == 20) then	
		Achievements:Award("Database")
	end
	if (self.numDiscoveries == #Arm.Discoveries) then
		Achievements:Award("Codex")
	end
	GameNetwork.LogEvent("Discovered: "..id)
end

function Achievements.Manipulated(self)
	self.numManipulated = self.numManipulated + 1
	Persistence.WriteNumber(SaveGame, "acv_manipulated", self.numManipulated)
	if (self.numManipulated == 10) then	
		Achievements:Award("Telekinesis")
	end
	GameNetwork.LogEvent("Manipulated")
end

function Achievements.GlyphDiscovered(self)
	self.numGlyphs = self.numGlyphs + 1
	Persistence.WriteNumber(SaveGame, "acv_glyphs", self.numGlyphs)
	if (self.numGlyphs == 8) then	
		Achievements:Award("Symbolism")
	end
	GameNetwork.LogEvent("GlyphDiscovered")
end

function Achievements.SolvedPuzzle(self, level)
	Achievements:Award("Interface")
	if (level >= 4) then
		Achievements:Award("Einstein")
	end
	GameNetwork.LogEvent("Solved Level "..tostring(level).." Terminal", {map=World.worldspawn.keys.mappath})
end

function Achievements.HackedPuzzle(self, level)
	Achievements:Award("Interface")
	if (level >= 4) then
		Achievements:Award("Brawn")
	end
	GameNetwork.LogEvent("Hacked Level "..tostring(level).." Terminal", {map=World.worldspawn.keys.mappath})
end

function Achievements.Award(self, name, silent)

	local acv = Achievements.Ids[name]
	if (acv == nil) then
		HUD:Print(nil, "Error, no achivement named "..name..".", nil, false)
		return
	end

	local awarded = Persistence.ReadBool(SaveGame, "achievement", false, acv.Id)
	if (awarded) then
		return
	end
	
	Persistence.WriteBool(SaveGame, "achievement", true, acv.Id)
	
	local icon = {
		size = {
			48*UI.identityScale[1],
			48*UI.identityScale[1]
		},
		material = acv.Icon
	}
	
	if (not silent) then
		HUD:Print(icon, StringTable.Get("ACV_UNLOCKED_HUD"):format(StringTable.Get(acv.String), PlayerSkills.kAchievementReward), nil, false)
		PlayerSkills:AwardSkillPoints(PlayerSkills.kAchievementReward)
	end
	
	EventLog:AddEvent(GameDB:ArmDateString(), "!ACHIEVEMENT", acv.String)
	
	GameNetwork.SendAchievement(acv.Id)
	GameNetwork.LogEvent("Achievement: "..acv.Id)
	
	Achievements.Sound:Play(kSoundChannel_UI, 0)
end
