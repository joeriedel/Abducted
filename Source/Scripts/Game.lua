-- Game.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Game = Entity:New()

function Game.Spawn(self)
	Game.entity = self
	PostFX.Init()
	Input:Spawn()
	World.SetGameCode(self)
	World.gameCode = self
	World.gameTimers = TimerList:Create()
	World.globalTimers = TimerList:Create()
	DebugUI:Spawn()
end

function Game.Initialize(self, type)
	Game.type = type
	
	World.SetEnabledGestures(0)
	
	if (type == "Map" or type == "Cinematic") then
		Abducted:New(Game.entity)
		Game.entity:Initialize()
	elseif (type == "MainMenu") then
		MainMenu:New(Game.entity)
		Game.entity:Initialize()
	end
end

function Game.Think(self, dt)
	Game.time = World.GameTime()
	Game.sysTime = World.SysTime()
	
	if (not World.paused) then
		World.gameTimers:Tick(Game.time)
	end
	
	World.globalTimers:Tick(Game.sysTime)
	
	FlyCam:Think(dt)
	DebugUI:Think()
end

function Game.PostSpawn(self)
	
end

function Game.LoadLevel(self, level)

	Game.entity.eatInput = true
	UI:BlendTo({0,0,0,1}, 1)
	World.SoundFadeMasterVolume(0, 1)
	Persistence.WriteBool(Session, "loadCheckpoint", false)
	Session:Save()
	
	GameDB:SaveCheckpointTransition(level)
	
	local f = function()
		World.RequestLoad(level, kUnloadDisposition_Slot)
	end
	
	World.globalTimers:Add(f, 1)

end

function Game.OnEvent(self, cmd, args)

	COutLineEvent("Game", "@world", cmd, args)
	
	if cmd == "fadein" then
		UI:FadeIn(tonumber(args))
		return true
	elseif cmd == "fadetoblack" then
		UI:BlendTo({0, 0, 0, 1}, tonumber(args))
		return true
	elseif cmd == "fadetowhite" then
		UI:BlendTo({1, 1, 1, 1}, tonumber(args))
		return true
	elseif cmd == "maskfadein" then
		UI:MaskFadeIn(tonumber(args))
		return true
	elseif cmd == "maskfadetoblack" then
		UI:MaskBlendTo({0, 0, 0, 1}, tonumber(args))
		return true
	elseif cmd == "maskfadetowhite" then
		UI:MaskBlendTo({1, 1, 1, 1}, tonumber(args))
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
		return true
	elseif (cmd == "disablewaypoints") then
		Floors:SetWaypointTargetnameState(args, nil, bit.bnot(kWaypointState_Enabled))
		return true
	elseif (cmd == "enablewaypointuserid") then
		Floors:SetWaypointUserIdState(args, kWaypointState_Enabled)
		return true
	elseif (cmd == "disablewaypointuserid") then
		Floors:SetWaypointUserIdState(args, nil, bit.bnot(kWaypointState_Enabled))
		return true
	elseif (cmd == "enablefloor") then
		Floors:SetFloorState(args, kFloorState_Enabled)
		return true
	elseif (cmd == "disablefloor") then
		Floors:SetFloorState(args, nil, bit.bnot(kFloorState_Enabled))
		return true
	elseif (cmd == "play") then
		Cinematics:Play(args)
		return true
	elseif (cmd == "stop") then
		Cinematics:Stop(args)
		return true
	elseif (cmd == "music") then
		local x = Tokenize(args)
		cmd = x[1]
		table.remove(x, 1)
		Music.entity:Command(cmd, x, args)
		return true
	elseif (cmd == "signal_arm") then
		Arm:Signal(args)
		return true
	elseif (cmd == "disable_arm") then
		PlayerSkills:LockArm()
		return true
	elseif (cmd == "enable_arm") then
		PlayerSkills:UnlockArm()
		return true
	elseif (cmd == "unlock_topic") then
		self:UnlockTopic(args)
		return true
	elseif (cmd == "silent_unlock_topic") then
		self:UnlockTopic(args, true)
		return true
	elseif (cmd == "lock_topic") then
		self:LockTopic(args)
		return true
	elseif (cmd == "horror_topic") then
		Arm:HorrorTopic(args)
		return true
	elseif (cmd == "enable_manipulate") then
		PlayerSkills:UnlockManipulate()
		return true
	elseif (cmd == "enable_shield") then
		PlayerSkills:UnlockShield()
		return true
	elseif (cmd == "enable_pulse") then
		PlayerSkills:UnlockPulse()
		return true
	elseif (cmd == "discover") then
		self:Discover(args)
		return true
	elseif (cmd == "checkpoint") then
		Abducted.entity:VisibleCheckpoint()
		return true
	elseif (cmd == "mainMenuCommand") then
		if (self.OnMainMenuCommand) then
			self:OnMainMenuCommand(cmds, args)
		end
		return true
	elseif (cmd == "hudprint") then
		HUD:Print(nil, args)
		return true
	elseif (cmd == "log") then
		if (args) then
			HUD:Print(nil, "HUD_LOG_UPDATED")
			EventLog:AddEvent(GameDB:ArmDateString(), "!EVENT", args)
		else
			COutLine(kC_Debug, "@world log command requires string argument!")
		end
		return true
	elseif (cmd == "load") then
		self:LoadLevel(args)
		return true
	elseif (cmd == "achievement") then
		if (args) then
			Achievements:Award(args)
			return true
		end
	elseif (cmd == "downloadbar") then
		if (args) then
			HUD:DoDownloadBar(tonumber(args))
			return true
		end
	elseif (cmd == "endep1") then
		Abducted.entity:EndEpisode(1)
		return true
	end
end

game_code = Game

function World.BuiltIns() -- before music starts
	return {
		"imuse"
	}
end

function World.BuiltIns2()
	return {
		"ui_code",
		"game_code",
		"scexec_code",
		"memory_game",
        "reflex_game"
	}
end
