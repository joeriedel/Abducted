-- Abducted.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Abducted = Game:New()
Abducted.PulseTargets = LL_New()

function Abducted.Initialize(self)
	self.eatInput = false
	
	GameDB:Load()
	PlayerSkills:Load()
	Abducted:Load()
	PlayerInput:Spawn()
	HUD:Spawn()
	Arm:Spawn()
	TerminalScreen.StaticInit()
	
	self.think = Abducted.Think
	self:SetNextThink(0)
	
	self.manipulate = false
end

function Abducted.PostSpawn(self)
	
	if (GameDB:LoadingSaveGame()) then
		self:LoadCheckpoint()
	else
		self:SaveCheckpoint()
	end
	
end

function Abducted.LoadCheckpoint(self)
	World.gameTimers = TimerList:Create()
	World.globalTimers = TimerList:Create()
	GameDB:LoadCheckpoint()
end

function Abducted.SaveCheckpoint(self)
	if (not World.playerPawn.dead) then
		GameDB:SaveCheckpoint()
	end
end

function Abducted.LoadState(self)
	HUD:LoadState()
end

function Abducted.SaveState(self)
	HUD:SaveState()
end

function Abducted.Load(self)
	self.gfx = {}
	self.gfx.Manipulate = World.Load("Shared/spellcasting_edgelines_M")
	
	self.sfx = {}
	self.sfx.ManipulateBegin = World.LoadSound("Audio/manipulate_start")
	self.sfx.ManipulateEnd = World.LoadSound("Audio/manipulate_end")
	
	self.overlays = {}
	self.overlays.Manipulate = World.CreateScreenOverlay(self.gfx.Manipulate)
end

function Abducted.InputEventFilter(self, e)
	if (e.type == kI_KeyDown) then
		if (e.data[1] == kKeyCode_P) then
			World.PauseGame(not World.paused)
		end
	end
	if (self.eatInput) then
		return true
	end
	if (Cinematics.busy > 0) then
		return true
	end
	
	return false
end

function Abducted.InputGestureFilter(self, g)
	return false
end

function Abducted.OnInputEvent(self, e)
	if (Arm.active) then
		return false
	end
	if (TerminalPuzzles.active) then
		return false
	end
	if (self.manipulate) then
		return false
	end
	if (TerminalPuzzles.active) then
		return false
	end
	if (TerminalScreen.Touch(e)) then
		return true
	end
	
	local handled, action = PlayerInput:OnInputEvent(e)
	return handled
end

function Abducted.OnInputGesture(self, g)
	if (Arm.active) then
		return false
	end
	
	if (TerminalPuzzles.active) then
		return false
	end
	
	if (self.manipulate) then
		if (g.id == kIG_Line) then
			if (ManipulatableObject.ManipulateGesture(g)) then
				self:EndManipulate()
				HUD:RechargeManipulate()
			end
		end
		return true
	end
	
	return PlayerInput:OnInputGesture(g)
end

function Abducted.BeginManipulate(self)
	if (self.manipulate) then
		self:EndManipulate()
		return
	end
	self.overlays.Manipulate:FadeIn(0.15)
	self.sfx.ManipulateBegin:FadeVolume(1, 0)
	self.sfx.ManipulateBegin:Rewind()
	self.sfx.ManipulateBegin:Play(kSoundChannel_UI, 0)
	self.manipulate = true
	ManipulatableObject.NotifyManipulate(true)
	World.playerPawn:NotifyManipulate(true)
	World.SetEnabledGestures(kIG_Line)
	World.FlushInput(true)
	
	local f = function ()
		self:EndManipulate()
	end
	
	self.endManipulateTimer = World.gameTimers:Add(f, 1.8, true)
	
	f = function ()
		World.SetGameSpeed(0.2, 0.5)
	end
	
	self.setGameSpeedTimer = World.gameTimers:Add(f, 0.7, true)
end

function Abducted.EndManipulate(self, immediate)
	
	if (immediate) then
		World.SetGameSpeed(1, 0)
		self.overlays.Manipulate:FadeOut(0.5)
		self.sfx.ManipulateBegin:FadeOutAndStop(0.5)
	else
		World.SetGameSpeed(1, 0.5)
		self.overlays.Manipulate:FadeOut(0.5)
		self.sfx.ManipulateBegin:FadeOutAndStop(0.5)
	end
	
	self.sfx.ManipulateEnd:Play(kSoundChannel_UI, 0)
	self.manipulate = false
	ManipulatableObject.NotifyManipulate(false)
	World.playerPawn:NotifyManipulate(false)
	World.SetEnabledGestures(0)
	World.FlushInput(true)
	
	self.endManipulateTimer:Clean()
	self.setGameSpeedTimer:Clean()
end

function Abducted.BeginPulse(self)
	if (self.pulse) then
		self:EndPulse()
		return
	end
	
	self.pulse = true
	World.playerPawn:BeginPulse(self)
	
	local f = function()
		self:DischargePulse()
	end
	
	self.pulseTimer = World.gameTimers:Add(f, FloatRand(PlayerSkills.PulseExplodeTime[1], PlayerSkills.PulseExplodeTime[2]), true)
	
end

function Abducted.EndPulse(self)
	if (self.pulseTimer) then
		self.pulseTimer:Clean()
		self.pulseTimer = nil
	end
	
	World.playerPawn:EndPulse()
	self.pulse = false
end

function Abducted.DischargePulse(self)
	World.playerPawn:DischargePulse()
	self.pulse = false
end

function Abducted.FirePulse(self, target, normal)
	if (self.pulseTimer) then
		self.pulseTimer:Clean()
		self.pulseTimer = nil
	end
	
	World.playerPawn:FirePulse(target, normal)
	
	self.pulse = false
end

function Abducted.PlayerDied(self)
	HUD:PlayerDied()
	if (self.manipulate) then
		self:EndManipulate()
	end
	if (self.pulse) then
		self:EndPulse()
	end
	
	AlertPanel:Run(
		"DEFAULT_KILLED_MESSAGE",
		{
			{"ALERT_PANEL_BUTTON_RELOAD_CHECKPOINT", r=1},
			{"ALERT_PANEL_BUTTON_QUIT_TO_MAIN_MENU", r=2}
		}
	)
end

function Abducted.Think(self, dt)
	Game.Think(self, dt)
	
	if (self.lastSysTime == nil) then
		self.lastSysTime = self.sysTime
	end
	
	dt = self.sysTime - self.lastSysTime
	self.lastSysTime = self.sysTime
	
	GameDB:IncrementTime(dt)
	
	if (Arm.think) then
		Arm:think(dt)
	end
	
	TerminalScreen.UpdateUI()
end