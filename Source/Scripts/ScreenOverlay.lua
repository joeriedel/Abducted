-- ScreenOverlay.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ScreenOverlay = Entity:New()

function ScreenOverlay.Spawn(self)

	self.on = StringForString(self.keys.initial_state, "on") == "on"
	
	local material = World.Load(self.keys.material)
	self.overlay = World.CreateScreenOverlay(material)
	
	if (self.on) then
		self.overlay:FadeIn(0)
	end

	local io = {
		Save = function()
			return self:SaveState()
		end,
		Load = function(s, x)
			return self:LoadState(x)
		end
	}
	
	GameDB.PersistentObjects[self.keys.uuid] = io
	
end

function ScreenOverlay.OnEvent(self, cmd, args)

	COutLineEvent("ScreenOverlay", self.keys.targetname, cmd, args)
	
	if ((cmd == "fadein") and args) then
		self.on = true
		self.overlay:FadeIn(tonumber(args))
		return true
	elseif ((cmd == "fadeout") and args) then
		self.on = false
		self.overlay:FadeOut(tonumber(args))
		return true
	end

	return false
	
end

function ScreenOverlay.SaveState(self)
	local state = {
		on = tostring(self.on)
	}
	return state
end

function ScreenOverlay.LoadState(self, state)
	self.on = state.on == "true"
	
	if (self.on) then
		self.overlay:FadeIn(0)
	else
		self.overlay:FadeOut(0)
	end
end

info_screenoverlay = ScreenOverlay