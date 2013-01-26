-- Cinematics.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Cinematics = Class:New()

function Cinematics.Play(self, args)
	if (self.busy == nil) then
		self.busy = 0
	end
	
	args = Tokenize(args)
	local animateCamera = not FindArrayElement(args, "camera=no")
	local flags = 0
	
	if (animateCamera) then
		flags = bit.bor(flags, kCinematicFlag_AnimateCamera)
	end
	
	if (World.PlayCinematic(args[1], flags, 0, Game.entity, Cinematics.Callbacks)) then
		if (self.busy == 0) then
			HUD:SetVisible(false)
		end
		self.busy = self.busy + 1
	end
end

function Cinematics.PlayLevelCinematics(self)
	World.PlayCinematic("environment", kCinematicFlag_CanPlayForever, 0)
	World.PlayCinematic("environment_loop", kCinematicFlag_Loop, 0)
	Cinematics:Play("intro")
end

Cinematics.Callbacks = {}

function Cinematics.Callbacks.OnTag(self, tag)
	World.PostEvent(tag)
end

function Cinematics.Callbacks.OnComplete(self)
	Cinematics.busy = Cinematics.busy - 1
	if (Cinematics.busy == 0) then
		HUD:SetVisible(true)
	end
end