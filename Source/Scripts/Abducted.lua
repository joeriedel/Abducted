-- Abducted.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Abducted = Game:New()
Abducted.PulseTargets = LL_New()
Abducted.KillMessages = { "DEFAULT_KILLED_MESSAGE1" }

function Abducted.Initialize(self)
	math.randomseed(System.ReadMilliseconds())
	
	self.eatInput = false
	Abducted.KeyBindings = LoadKeyBindings()
	
	UI:InitMap()
	GameDB:Load()
	PlayerSkills:Load()
	Abducted:Load()
	PlayerInput:Spawn()
	HUD:Spawn()
	Arm:Spawn()
	ManipulatableObjectUI:Spawn()
	TerminalScreen.StaticInit()
	
	self.think = Abducted.Think
	self:SetNextThink(0)
	
	self.manipulate = false
end

function Abducted.PostSpawn(self)
	Game.PostSpawn(self)
	Cinematics:PlayLevelCinematics()
end

function Abducted.OnLevelStart(self)
	
	if (GameDB:LoadingSaveGame()) then
		COutLine(kC_Debug, "Loading checkpoint!")
		self:LoadCheckpoint()
	else
		local f = function()
			self.showCheckpointNotification = false
			World.RequestGenerateSaveGame()
		end
		World.globalTimers:Add(f, 0.25) -- let scripts and stuff fire to set states
	end
	
end

function Abducted.VisibleCheckpoint(self)
	self.showCheckpointNotification = true
	World.RequestGenerateSaveGame()
end

function Abducted.LoadCheckpoint(self)
	World.gameTimers = TimerList:Create()
	World.globalTimers = TimerList:Create()
	World.gameTimers.time = Game.time or 0
	World.globalTimers.time = Game.sysTime or 0
	GameDB:LoadCheckpoint()
end

function Abducted.SaveCheckpoint(self)
	if (not World.playerPawn.dead) then
		GameDB:SaveCheckpoint()
	end
end

function Abducted.LoadState(self)

	World.SetGameSpeed(1, 0)
	self.overlays.Manipulate:FadeOut(0)
	self.sfx.ManipulateBegin:Stop()
	self.manipulate = false
	World.SetEnabledGestures(0)
	World.FlushInput(true)
	self.pulse = false

	PlayerSkills:Load()
	HUD:LoadState()
	Arm:LoadState()
	Cinematics:LoadState()
	Floors:LoadState()
end

function Abducted.SaveState(self)
	PlayerSkills:Save()
	HUD:SaveState()
	Arm:SaveState()
	Cinematics:SaveState()
	Floors:SaveState()
end

function Abducted.DoLoadCheckpoint(self)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.5)
	
	local f = function ()
		self:LoadCheckpoint()
		UI:BlendTo({1,1,1,0}, 0.5)
		Abducted.entity.eatInput = false
	end

	World.globalTimers:Add(f, 0.5)
	
end

function Abducted.Load(self)
	self.gfx = {}
	self.gfx.Manipulate = World.Load("Shared/spellcasting_edgelines_M")
	self.gfx.PainBlend = World.Load("Shared/painblend_M")
		
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
	if (MemoryGame.active) then
		return false
    end
    if (ReflexGame.active) then
        return false
    end
    if (TerminalScreen.Active) then
		return false
	end
	local handled = self:InputKeyAction(e)
	if (handled) then
		return true
	end
	if (self.manipulate) then
		return false
	end
	local handled, action = PlayerInput:OnInputEvent(e)
	return handled
end

function Abducted.OnInputGesture(self, g)
	if (Arm.active) then
		return false
	end
	if (MemoryGame.active) then
		return false
    end
    if (ReflexGame.active) then
        return false
    end
	if (TerminalScreen.Active) then
		return false
	end
	
	if (self.manipulate) then
		if (g.id == kIG_Line) then
			ManipulatableObject.ManipulateGesture(g)
		end
		return true
	end
	
	return PlayerInput:OnInputGesture(g)
end

function Abducted.InputKeyAction(self, e)
	if (UI.mode == kGameUIMode_Mobile) then
		return false
	end
	
	if (e.type == kI_KeyUp) then
		return true
	end
	if (e.type ~= kI_KeyDown) then
		return false
	end
	
	local action = Abducted.KeyBindings.KeyToAction[e.data[1]]
	if (action == nil) then
		return false
	end
	
	if (action == kAction_Stop) then
		World.playerPawn:Stop()
		return true
	end
	
	if (ManipulatableObjectUI:HandleAction(action)) then
		return true
	end
	
	if (HUD:HandleAction(action)) then
		return true
	end

	return true
end

function Abducted.BeginManipulate(self)
	if (World.playerPawn.customMove) then
		return -- busy
	end
	
	if (self.manipulate) then
		self:EndManipulate()
		return
	end
	
	self.overlays.Manipulate:FadeIn(0.15)
	self.sfx.ManipulateBegin:FadeVolume(1, 0)
	self.sfx.ManipulateBegin:Rewind()
	self.sfx.ManipulateBegin:Play(kSoundChannel_UI, 0)
	self.manipulate = true
	self.numAvailableManipulateActions = PlayerSkills:NumManipulateActions()
	ManipulatableObject.NotifyManipulate(true)
	World.playerPawn:NotifyManipulate(true)
	
	if (UI.mode == kGameUIMode_Mobile) then
		World.SetEnabledGestures(kIG_Line)
	end
	
	World.FlushInput(true)
	
	self:ExtendManipulateWindow()
	
	local f = function ()
		World.SetGameSpeed(1, 0)
	end
	
	self.setGameSpeedTimer = World.gameTimers:Add(f, 0.7)
	HUD:RefreshAvailableActions()
end

function Abducted.ExtendManipulateWindow(self)

	if (self.endManipulateTimer) then
		self.endManipulateTimer:Clean()
	end
	
	local f = function ()
		self:EndManipulate()
	end
	
	self.endManipulateTimer = World.gameTimers:Add(f, 1.8)

end

function Abducted.EndManipulate(self, immediate)
	
	if (immediate) then
		World.SetGameSpeed(1, 0.2)
		self.overlays.Manipulate:FadeOut(0.5)
		self.sfx.ManipulateBegin:FadeOutAndStop(0.5)
	else
		World.SetGameSpeed(1, 0.2)
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
	HUD:RefreshAvailableActions()
end

function Abducted.BeginPulse(self)
	if (World.playerPawn.customMove) then
		return -- busy
	end
	
	if (self.pulse) then
		self:EndPulse()
		return
	end
	
	self.pulse = true
	HUD:ShowPulseModes(true)
	HUD:RefreshAvailableActions()
	World.playerPawn:BeginPulse(self)
	
	local f = function()
		self:DischargePulse()
	end
	
	self.pulseTimer = World.gameTimers:Add(f, FloatRand(PlayerSkills.PulseExplodeTime[1], PlayerSkills.PulseExplodeTime[2]))
	
end

function Abducted.EndPulse(self)
	if (self.pulseTimer) then
		self.pulseTimer:Clean()
		self.pulseTimer = nil
	end
	
	World.playerPawn:EndPulse()
	self.pulse = false
	
	HUD:ShowPulseModes(false)
	HUD:RefreshAvailableActions()
end

function Abducted.DischargePulse(self)
	local exploded, fired = World.playerPawn:DischargePulse()
	self.pulse = false
	
	if (fired) then
		HUD:ShieldPenalty()
	end
	
	HUD:ShowPulseModes(false)
	
	if (not exploded) then
		HUD:RefreshAvailableActions()
	end
end

function Abducted.FirePulse(self, target, normal)
	if (self.pulseTimer) then
		self.pulseTimer:Clean()
		self.pulseTimer = nil
	end
	
	World.playerPawn:FirePulse(target, normal)
	World.playerPawn:PulseDamage(target)
	
	self.pulse = false
	HUD:ShieldPenalty()
	HUD:ShowPulseModes(false)
	HUD:RefreshAvailableActions()
end

function Abducted.PowerBubble(self)
	if (self.pulseTimer) then
		self.pulseTimer:Clean()
		self.pulseTimer = nil
	end
		
	local f = function()
		World.playerPawn:PowerBubble()
	end
	
	if (not World.playerPawn.shieldActive) then
		HUD:BeginShield()
		World.gameTimers:Add(f, 0.1)
	else
		f()
	end
	
	self.pulse = false
	HUD:ShowPulseModes(false)
	HUD:RefreshAvailableActions()
end

function Abducted.PlayerDiedAlertPanelDone(self, result)
	if (result == AlertPanel.YesButton) then
		self:DoLoadCheckpoint()
	else
		Abducted.entity.eatInput = true
		UI:BlendTo({0,0,0,0}, 0)
		UI:BlendTo({0,0,0,1}, 1)
		World.SoundFadeMasterVolume(0, 1)
		local f = function()
			World.RequestLoad("UI/mainmenu", kUnloadDisposition_Slot)
		end
		World.globalTimers:Add(f, 1.1)
	end
end

function Abducted.PlayerDied(self, killMessage)
	HUD:PlayerDied()
	if (self.manipulate) then
		self:EndManipulate()
	end
	if (self.pulse) then
		self:EndPulse()
	end
	
	if (killMessage == nil) then
		killMessage = Abducted.KillMessages[IntRand(1, #Abducted.KillMessages)]
	end
	
	local f = function()
		AlertPanel:Run(
			"ALERT_PANEL_DIED",
			killMessage,
			{
				{"ALERT_PANEL_BUTTON_RELOAD_CHECKPOINT", r=1},
				{"ALERT_PANEL_BUTTON_QUIT_TO_MAIN_MENU", r=2}
			},
			function (result)
				self:PlayerDiedAlertPanelDone(result)
			end
		)
	end
	
	World.globalTimers:Add(f, 2.5)
end

function Abducted.Discover(self, name)
	return GameDB:Discover(name)
end

function Abducted.UnlockTopic(self, name)
	Arm:UnlockTopic(name)
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
	
	ManipulatableObjectUI:UpdateUI()
	TerminalScreen.UpdateUI()
	TerminalScreen.CheckActivate(dt)
end