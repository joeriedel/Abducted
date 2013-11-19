-- GameDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

GameDB = Class:New()
GameDB.SaveGameVersion = 1
GameDB.SaveGameMinVersion = 1
GameDB.GameSecondsPerSecond = 8 -- means 4 hours of gameplay = 1 day
GameDB.SecondsPerMinute = 60
GameDB.MinutesPerHour = 60
GameDB.HoursPerDay = 24
GameDB.SecondsPerHour = 60*60
GameDB.SecondsPerDay = 60*60*24
GameDB.AllDiscoveriesCheat = false
GameDB.PersistentObjects = {}
GameDB.Portraits = {
	"UI/portrait1_M",
	"UI/portrait2_M",
	"UI/portrait3_M",
	"UI/portrait4_M"
}

function GameDB.ValidCheckpoint(self)

	-- we may have old checkpoint data from a previously loaded map if
	-- we crashed or otherwise were interrupted before the intro cinematics
	-- of this map completed.
	--
	-- In this case we never got passed that point so don't load checkpoint state
	-- from an old map into this one!
	
	local checkpointmap = Persistence.ReadString(SaveGame, "checkpointmap")
	return checkpointmap and (checkpointmap == World.worldspawn.keys.mappath)
	
end

function GameDB.Load(self)

	self.playerName = Persistence.ReadString(SaveGame, "playerName", "Eve")
	self.playerStyle = Persistence.ReadNumber(SaveGame, "portrait", 1)
	self.portrait = GameDB.Portraits[self.playerStyle]
	if (self.portrait == nil) then
		self.portrait = GameDB.Portraits[1]
	end
	self.numDiscoveries = Persistence.ReadNumber(SaveGame, "numDiscoveries", 0)
	self.discoveryTime = Persistence.ReadNumber(SaveGame, "lastDiscoveryTime", 0)
	self.bugKillCounter = Persistence.ReadNumber(SaveGame, "bugKillCounter", 0)
	self.loadingCheckpoint = Persistence.ReadBool(Session, "loadCheckpoint", false)
		
	Persistence.WriteBool(Session, "loadCheckpoint", false)
	Session:Save()
	
	self.loadingCheckpoint = self.loadingCheckpoint and self:ValidCheckpoint()
	self.bugSquishCounter = 0
	
	self:LoadTime()
	self:LoadChatLockouts()
	EventLog:Load()
	
	-- check bug kill counter
	local f = function()
		self:CheckBugKillCounter()
	end
	
	World.gameTimers:Add(f, 25, true)
	
	local f = function()
		self:CheckBugSquishCounter()
	end
	
	World.gameTimers:Add(f, 0, true)
end

function GameDB.CheckBugKillCounter(self)
	if (self.bugKillCounter > 0) then
		EventLog:AddEvent(GameDB:ArmDateString(), "!KILLEDBUGS", tostring(self.bugKillCounter))
		self.bugKillCounter = 0
	end
end

function GameDB.CheckBugSquishCounter(self)
	if (self.bugSquishCounter > 0) then
		EventLog:AddEvent(GameDB:ArmDateString(), "!SQUISHEDBUGS", tostring(self.bugSquishCounter))
		self.bugSquishCounter = 0
	end
end

function GameDB.LoadingSaveGame(self)
	return self.loadingCheckpoint
end

function GameDB.SaveCheckpoint(self)
	Persistence.WriteNumber(SaveGame, "secondsPlayed", self.realTime)
	Persistence.WriteNumber(SaveGame, "bugKillCounter", self.bugKillCounter)
	Persistence.WriteString(SaveGame, "lastPlayed", CurrentDateAndTimeString())
	Persistence.WriteString(SaveGame, "checkpointmap", World.worldspawn.keys.mappath)
	Persistence.WriteNumber(SaveGame, "version", GameDB.SaveGameVersion)
	GameDB:SaveEvents()
	GameDB:SavePeristentObjects()
	Game.entity:SaveState()
	SaveGame:Save()
end

function GameDB.SaveCheckpointTransition(self, level)

	Persistence.WriteNumber(SaveGame, "secondsPlayed", self.realTime)
	Persistence.WriteNumber(SaveGame, "bugKillCounter", self.bugKillCounter)
	Persistence.WriteString(SaveGame, "lastPlayed", CurrentDateAndTimeString())
	Persistence.WriteString(SaveGame, "currentLevel", level)
	SaveGame:Save()
	
end

function GameDB.LoadCheckpoint(self)
	SaveGame:Load() -- load savegame data
	GameDB:Load()
	World.MarkTempEntsForGC()
	self.loadingCheckpoint = true
	TerminalScreen.CancelUI()
	Discovery.ResetUIForCheckpoint()
	Metadata.ResetForCheckpoint()
	PostFX.ResetForCheckpoint()
	Game.entity:LoadState()
	GameDB:LoadPersistentObjects()
	GameDB:LoadEvents()
	self.loadingCheckpoint = false
end

function GameDB.SaveEvents(self)

	local t = World.GetEvents() or {}
	
	Persistence.WriteNumber(SaveGame, "worldEventQueueSize", #t)
	
	for k,v in pairs(t) do
		Persistence.WriteString(SaveGame, "worldEventQueue", v, k)
	end

end

function GameDB.LoadEvents(self)

	World.FlushEvents()
	
	local n = Persistence.ReadNumber(SaveGame, "worldEventQueueSize", 0)
	
	for i = 1,n do
	
		local x = Persistence.ReadString(SaveGame, "worldEventQueue", nil, i)
		World.PostEvent(x)
	
	end

end

function GameDB.LoadPersistentObjects(self)

	World.viewController:FadeOutLookTargets(0)
	ViewController.Targets = {}
	
	local db = SaveGame.keys["persistentObjectData"]
	
	for k,v in pairs(GameDB.PersistentObjects) do
		assert(type(k) == "string")
		if (x ~= "vc") then
			local x = db[k]
			if (x) then
			-- new objects won't have state.
				v:Load(x)
			end
		end
	end
	
	local vc = GameDB.PersistentObjects["vc"]
	if (vc) then
		local x = db["vc"]
		if (x) then
			vc:Load(x)
		end
	end
	
	for k,v in pairs(GameDB.PersistentObjects) do
	
		if (v.Done) then
			v:Done()
		end
	
	end

end

function GameDB.SavePeristentObjects(self)

	local db = {}
	
	for k,v in pairs(GameDB.PersistentObjects) do
		assert(type(k) == "string")
		local x = v:Save()
		if (x) then
			db[k] = x
		end
		
	end
	
	SaveGame.keys["persistentObjectData"] = db

end

function GameDB.KilledBugs(self, num)
	self.bugKillCounter = self.bugKillCounter + num
end

function GameDB.SquishedBugs(self, num)
	self.bugSquishCounter = self.bugSquishCounter + num
end

function GameDB.LoadTime(self)

	self.realTime = Persistence.ReadNumber(SaveGame, "secondsPlayed", 0)
	self:UpdateTimes()
	
end

function GameDB.Discover(self, name, source, unlock, visible)

	local dbItem = Arm.Discoveries[name]
	if (not dbItem) then
		COutLine(kC_Error, "There is no '%s' in the database.", name)
		return false
	end

	local discovered = self:CheckDiscovery(name)
	if (unlock) then
		if (discovered == "unlocked") then
			return false
		end
	else
		if (discovered) then
			return false
		end
	end
	
	GameDB.discoveryTime = GameDB.time
	
	-- add log text
	local logTitle = nil
	local logEntry = nil

	if (unlock or (dbItem.mysteryTitle == nil)) then
		unlock = true -- not a mystery item, count the discovery
		logTitle = dbItem.title or "MISSING TITLE TEXT!"
		if (dbItem.logText) then
			logEntry = dbItem.logText[source] or dbItem.logText.all or "MISSING LOG TEXT!"
		else
			logEntry = "MISSING LOG TEXT!"
		end
		
		if (dbItem.events) then
			for k,v in pairs(dbItem.events) do
				World.PostEvent(v)
			end
		end
	else
		logTitle = dbItem.mysteryTitle
		logEntry = dbItem.mysteryLogText or "MISSING MYSTERY LOG TEXT!"
	end
	
	EventLog:AddEvent(
		GameDB:ArmDateString(), 
		"!DISCOVERY",
		name..";"..logEntry
	)
	
	if (visible) then
		local text = StringTable.Get("ARM_REWARD_DISCOVERY")..": "..StringTable.Get(logTitle)
		HUD:Print(nil, text, nil, false)
	end

	if (unlock) then
		self.numDiscoveries = self.numDiscoveries + 1
		Persistence.WriteNumber(SaveGame, "numDiscoveries", self.numDiscoveries)
		Persistence.WriteString(SaveGame, "discovery", "unlocked", name)
	else
		Persistence.WriteString(SaveGame, "discovery", "mystery", name)
	end
	
	self.discoveryTime = self.realTime
	Persistence.WriteNumber(SaveGame, "lastDiscoveryTime", self.discoveryTime)
	return true
	
end

function GameDB.CheckDiscovery(self, name)
	if (GameDB.AllDiscoveriesCheat) then
		return "unlocked"
	end
	return Persistence.ReadString(SaveGame, "discovery", nil, name)
end

function GameDB.CheckDiscoveryUnlocked(self, name)
	return self:CheckDiscovery(name) == "unlocked"
end

function GameDB.LoadChatLockouts(self)

	self.chatLockout = Persistence.ReadBool(SaveGame, "chatLockout", false)
	self.chatLockoutTime = Persistence.ReadNumber(SaveGame, "chatLockoutTime")

end

function GameDB.SaveChatLockouts(self)
	Persistence.WriteBool(SaveGame, "chatLockout", self.chatLockout)
	
	if (self.chatLockoutTime) then
		Persistence.WriteNumber(SaveGame, "chatLockoutTime", self.chatLockoutTime)
	else
		Persistence.DeleteKey(SaveGame, "chatLockoutTime")
	end
end

function GameDB.LockoutChat(self, time)

	self.chatLockout = true
	self.chatLockoutTime = nil
	
	if (time) then
		self.chatLockoutTime = self.realTime + time
	end
	
	self:SaveChatLockouts()
	
end

function GameDB.CheckChatLockout(self)

	if (self.chatLockout and self.chatLockoutTime) then
		if (self.realTime >= self.chatLockoutTime) then
			self.chatLockoutTime = nil
			self.chatLockout = false
			self:SaveChatLockouts()
		end
	end

end

function GameDB.ClearChatLockout(self)
	self.chatLockout = false
	self.chatLockoutTime = nil
	self:SaveChatLockouts()
end

function GameDB.IncrementTime(self, dt)
-- GameDB.realTime tracks the actual elapsed time in a game. It is serialized
-- with checkpoints so it should be used as the time base for anything that needs
-- to persist.
	self.realTime = self.realTime + dt
	self:UpdateTimes()
end

function GameDB.UpdateTimes(self)
	self.time = self.realTime * GameDB.GameSecondsPerSecond
	
	self.seconds = self.time
	self.days = math.floor(self.seconds / GameDB.SecondsPerDay)
	self.seconds = self.seconds - (self.days * GameDB.SecondsPerDay)
	self.hours = math.floor(self.seconds / GameDB.SecondsPerHour)
	self.seconds = self.seconds - (self.hours * GameDB.SecondsPerHour)
	self.minutes = math.floor(self.seconds / GameDB.SecondsPerMinute)
	self.seconds = self.seconds - (self.minutes * GameDB.SecondsPerMinute)
	
end

function GameDB.ArmDateString(self)
	return string.format("%03d-%02d:%02d:%02d", self.days, self.hours, self.minutes, self.seconds)
end

function GameDB.TimePlayedString(self)
	local seconds = self.realTime
	
	local hours = math.floor(seconds / GameDB.SecondsPerHour)
	local seconds = seconds - (hours * GameDB.SecondsPerHour)
	local minutes = math.floor(seconds / GameDB.SecondsPerMinute)
	seconds = seconds - (minutes * GameDB.SecondsPerMinute)
	
	return string.format("%02d:%02d:%02d", hours, minutes, seconds)
	
end

