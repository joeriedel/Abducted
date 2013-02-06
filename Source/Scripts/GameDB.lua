-- GameDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

GameDB = Class:New()
GameDB.GameSecondsPerSecond = 8 -- means about 4 hours of gameplay = 1 day
GameDB.SecondsPerMinute = 60
GameDB.MinutesPerHour = 60
GameDB.HoursPerDay = 24
GameDB.SecondsPerHour = 60*60
GameDB.SecondsPerDay = 60*60*24

function GameDB.Load(self)

	self.playerName = Persistence.ReadString(SaveGame, "playerName", "Eve")
	self.portrait = Persistence.ReadString(SaveGame, "portrait", "UI/character-profiletest1_M")
	self.numDiscoveries = Persistence.ReadNumber(SaveGame, "numDiscoveries", 0)
	self.discoveryTime = Persistence.ReadNumber(SaveGame, "lastDiscoveryTime", 0)
	
	self:LoadTime()
	self:LoadChatLockouts()
	EventLog:Load()
	
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
	
	SaveGame:Save()

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
	
	SaveGame:Save()
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
	Persistence.WriteNumber(SaveGame, "secondsPlayed", self.realTime)
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

function GameDB.CurrentTimeString(self)
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


