-- Game.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Game = Class:New()

function Game.Spawn(self)
	Game.entity = self
	PlayerInput:Spawn()
end

function Game.PostSpawn(self)
end

function Game.OnEvent(self, cmd, args)
	if cmd == "fadein" then
		UI:FadeIn(tonumber(args))
		return true
	elseif cmd == "fadetoblack" then
		UI:FadeToColor({0, 0, 0, 1}, tonumber(args))
		return true
	elseif cmd == "fadetowhite" then
		UI:FadeToColor({1, 1, 1, 1}, tonumber(args))
		return true
	elseif cmd == "fadeinletterbox" then
		UI:FadeInLetterBox({0, 0, 0, 1}, tonumber(args))
		return true
	elseif cmd == "fadeoutletterbox" then
		UI:FadeOutLetterBox(tonumber(args))
		return true
	elseif cmd == "soundfadeout" then
		World.SoundFadeMasterVolume(0, tonumber(args))
		return true
	elseif cmd == "soundfadein" then
		World.SoundFadeMasterVolume(1, tonumber(args))
		return true
	elseif (cmd == "enablewaypoints") then
		Floors.SetWaypointTargetnameState(args, kWaypointState_Enabled)
	elseif (cmd == "disablewaypoints") then
		Floors.SetWaypointTargetnameState(args, nil, bit.bnot(kWaypointState_Enabled))
	elseif (cmd == "enablewaypointuserid") then
		Floors.SetWaypointUserIdState(args, kWaypointState_Enabled)
	elseif (cmd == "disablewaypointuserid") then
		Floors.SetWaypointUserIdState(args, nil, bit.bnot(kWaypointState_Enabled))
	end
end

game_code = Game

function World.BuiltIns()
	return {
		"game_code",
		"ui_code",
		"scexec_code"
--		"terminal_puzzles"
	}
end