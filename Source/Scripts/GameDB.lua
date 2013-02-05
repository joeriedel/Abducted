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
	
	self:LoadTime()
	EventLog:Load()
	
end

function GameDB.LoadTime(self)

	self.realSecondsPlayed = Persistence.ReadNumber(SaveGame, "secondsPlayed", 0)
	self:UpdateTimes()
	
end

function GameDB.IncrementTime(self, dt)
	self.realSecondsPlayed = self.realSecondsPlayed + dt
	Persistence.WriteNumber(SaveGame, "secondsPlayed", self.realSecondsPlayed)
	self:UpdateTimes()
end

function GameDB.UpdateTimes(self)
	self.secondsPlayed = self.realSecondsPlayed * GameDB.GameSecondsPerSecond
	
	self.seconds = self.secondsPlayed
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
	local seconds = self.realSecondsPlayed
	
	local hours = math.floor(seconds / GameDB.SecondsPerHour)
	local seconds = seconds - (hours * GameDB.SecondsPerHour)
	local minutes = math.floor(seconds / GameDB.SecondsPerMinute)
	seconds = seconds - (minutes * GameDB.SecondsPerMinute)
	
	return string.format("%02d:%02d:%02d", hours, minutes, seconds)
	
end


