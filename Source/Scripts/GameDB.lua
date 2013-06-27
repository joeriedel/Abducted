-- GameDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

GameDB = Class:New()
GameDB.GameSecondsPerSecond = 8 -- means 4 hours of gameplay = 1 day
GameDB.SecondsPerMinute = 60
GameDB.MinutesPerHour = 60
GameDB.HoursPerDay = 24
GameDB.SecondsPerHour = 60*60
GameDB.SecondsPerDay = 60*60*24
GameDB.PersistentObjects = {}
GameDB.Portraits = {
	"UI/portrait1_M",
	"UI/portrait2_M",
	"UI/portrait3_M",
	"UI/portrait4_M"
}
	
function GameDB.Load(self)

	self.playerName = Persistence.ReadString(SaveGame, "playerName", "Eve")
	self.portrait = Persistence.ReadNumber(SaveGame, "portrait", 1)
	self.portrait = GameDB.Portraits[self.portrait]
	if (self.portrait == nil) then
		self.portrait = GameDB.Portraits[1]
	end
	self.numDiscoveries = Persistence.ReadNumber(SaveGame, "numDiscoveries", 0)
	self.discoveryTime = Persistence.ReadNumber(SaveGame, "lastDiscoveryTime", 0)
	self.loadingCheckpoint = Persistence.ReadBool(Session, "loadCheckpoint", false)
	
	self:LoadTime()
	self:LoadChatLockouts()
	EventLog:Load()
	
end

function GameDB.LoadingSaveGame(self)
	return self.loadingCheckpoint
end

function GameDB.SaveCheckpoint(self)
	Persistence.WriteNumber(SaveGame, "secondsPlayed", self.realTime)
	Persistence.WriteString(SaveGame, "lastPlayed", CurrentDateAndTimeString())
	GameDB:SaveEvents()
	GameDB:SavePeristentObjects()
	Game.entity:SaveState()
	SaveGame:Save()
end

function GameDB.LoadCheckpoint(self)
	World.MarkTempEntsForGC()
	GameDB:Load()
	Persistence.WriteBool(Session, "loadCheckpoint", false)
	Session:Save()
	self.loadingCheckpoint = true
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
			v:Load(x)
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

function GameDB.LoadTime(self)

	self.realTime = Persistence.ReadNumber(SaveGame, "secondsPlayed", 0)
	self:UpdateTimes()
	
end

function GameDB.Discover(self, name)

	local discovered = self:CheckDiscovery(name)
	if (discovered) then
		return
	end

	self.numDiscoveries = self.numDiscoveries + 1
	Persistence.WriteNumber(SaveGame, "numDiscoveries", self.numDiscoveries)
	Persistence.WriteBool(SaveGame, "discovery", true, name)
	
	self.discoveryTime = self.realTime
	Persistence.WriteNumber(SaveGame, "lastDiscoveryTime", self.discoveryTime)

end

function GameDB.CheckDiscovery(self, name)
	return true --Persistence.ReadBool(SaveGame, "discovery", false, name)
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

