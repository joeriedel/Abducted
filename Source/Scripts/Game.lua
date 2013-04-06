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
	DebugUI:Spawn()
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
	Game.time = World.GameTime()
	Game.sysTime = World.SysTime()
	
	if (not World.paused) then
		World.gameTimers:Tick(Game.time)
	end
	
	World.globalTimers:Tick(Game.sysTime)
end

function Game.PostSpawn(self)
	Cinematics:PlayLevelCinematics()
end

function Game.OnEvent(self, cmd, args)

	COutLineEvent("Game", cmd, args)
	
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
	elseif (cmd == "enablefloor") then
		Floors:SetFloorState(args, kFloorState_Enabled)
	elseif (cmd == "disablefloor") then
		Floors:SetFloorState(args, nil, bit.bnot(kFloorState_Enabled))
	elseif (cmd == "play") then
		Cinematics:Play(args)
	elseif (cmd == "music") then
		args = Tokenize(args)
		cmd = args[1]
		table.remove(args, 1)
		Music.entity:Command(cmd, args)
	end
end

game_code = Game

function World.BuiltIns()
	return {
		"imuse",
		"ui_code",
		"game_code",
		"scexec_code",
		"memory_game",
        "reflex_game"
	}
end
