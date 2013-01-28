-- Game.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Game = Entity:New()

function Game.Spawn(self)
	Game.entity = self
	World.SetGameCode(self)
	World.gameCode = self
	World.gameTimers = TimerList:Create()
	World.globalTimers = TimerList:Create()
end

function Game.Initialize(self, type)
	Game.type = type
	
	World.SetEnabledGestures(0)
	
	if (type == "Map") then
		Abducted:New(Game.entity)
		Game.entity:Initialize()
	end
end


function Game.Think(self)
	local time = World.GameTime()
	local sysTime = World.SysTime()
	
	if (not World.paused) then
		World.gameTimers:Tick(time)
	end
	
	World.globalTimers:Tick(sysTime)
end

function Game.PostSpawn(self)
	Cinematics:PlayLevelCinematics()
end

function Game.OnEvent(self, cmd, args)

	if (args) then
		COutLine(kC_Debug, "Game.OnEvent(%s, %s)", cmd, args)
	else
		COutLine(kC_Debug, "Game.OnEvent(%s)", cmd)
	end

	if cmd == "fadein" then
		UI:FadeIn(tonumber(args))
		return true
	elseif cmd == "fadetoblack" then
		UI:BlendTo({0, 0, 0, 1}, tonumber(args))
		return true
	elseif cmd == "fadetowhite" then
		UI:BlendTo({1, 1, 1, 1}, tonumber(args))
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
		Floors:SetWaypointTargetnameState(args, kWaypointState_Enabled)
	elseif (cmd == "disablewaypoints") then
		Floors:SetWaypointTargetnameState(args, nil, bit.bnot(kWaypointState_Enabled))
	elseif (cmd == "enablewaypointuserid") then
		Floors:SetWaypointUserIdState(args, kWaypointState_Enabled)
	elseif (cmd == "disablewaypointuserid") then
		Floors:SetWaypointUserIdState(args, nil, bit.bnot(kWaypointState_Enabled))
	elseif (cmd == "play") then
		Cinematics:Play(args)
	end
end

game_code = Game

function World.BuiltIns()
	return {
		"ui_code",
		"game_code",
		"scexec_code"
		"terminal_puzzles"
	}
end
