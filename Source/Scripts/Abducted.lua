-- Abducted.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Abducted = Game:New()

function Abducted.Initialize(self)
	self:Load()
	PlayerInput:Spawn()
	HUD:Spawn()
	PlayerSkills:Load()
	
	self.think = Abducted.Think
	self:SetNextThink(0)
	
	self.manipulate = false
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

function Abducted.OnInputEvent(self, e)
	if (self.manipulate) then
		return false
	end
	return PlayerInput:OnInputEvent(e)
end

function Abducted.OnInputGesture(self, g)
	if (self.manipulate) then
		if (g.id == kIG_Line) then
			if (ManipulatableObject.ManipulateGesture(g)) then
				self:EndManipulate()
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

function Abducted.EndManipulate(self)
	World.SetGameSpeed(1, 0.5)
	self.overlays.Manipulate:FadeOut(0.5)
	self.sfx.ManipulateBegin:FadeOutAndStop(0.5)
	self.sfx.ManipulateEnd:Play(kSoundChannel_UI, 0)
	self.manipulate = false
	ManipulatableObject.NotifyManipulate(false)
	World.playerPawn:NotifyManipulate(false)
	World.SetEnabledGestures(0)
	World.FlushInput(true)
	
	self.endManipulateTimer:Clean()
	self.setGameSpeedTimer:Clean()
end

function Abducted.Think(self, dt)
	Game.Think(self, dt)
end