-- Music.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Music = Class:New()

function Music.Spawn(self)
	Music.entity = self
	
	local io = {
		Save = function()
			return self:SaveState()
		end,
		Load = function(s, x)
			return self:LoadState(x)
		end
	}
	
	GameDB.PersistentObjects["music"] = io
end

function Music.Play(self)
	self.think = nil

	if (self.name and (self.name == self.args[1])) then
		-- already playing
		return
	end
	
	self:Stop() -- fade out active music
	self.active = World.Load(self.args[1])
	self.name = self.args[1]
	
	local fade = not FindArrayElement(self.args, "fade=false")
	local loop = FindArrayElement(self.args, "loop=true")
	
	if (fade) then
		self.active:FadeVolume(0, 0)
	end
	
	if (loop) then
		self.active:SetLoop(true)
	end
	
	self.active:Play(kSoundChannel_Music, 0)
	
	if (fade) then
		self.active:FadeVolume(1, 1)
	end
	
	self.args = nil
	self.think = Music.Check
	self:SetNextThink(1)
	
	World.PostEvent("shiphum fadeto 0.5 1")
end

function Music.Check(self)

	if (self.active) then
		if (not self.active:Playing()) then
			World.PostEvent("shiphum fadeto 1 1")
			self.think = nil
			self.active = nil
			self.name = nil
		end
	else
		self.think = nil
	end

end

function Music.Stop(self)
	self.name = nil
	if (self.active) then
		if (self.active:Playing()) then
			if (args == nil) then
				args = "1"
			end
			args = tonumber(args)
			local sound = self.active
			local f = function()
				sound = nil
			end
			self.active:FadeOutAndStop(args)
			self.active = nil
			World.gameTimers:Add(f, args+0.1, true)
		else
			self.active = nil
		end
	end
end

function Music.Command(self, cmd, args)
		
	if (cmd == "play") then
		self.args = args
		self.think = Music.Play
		self:SetNextThink(0)
		COutLine(kC_Debug, "Music.Command(%s, %s)", cmd, args[1])
		return true
	elseif (cmd == "stop") then
		self:Stop()
		COutLine(kC_Debug, "Music.Command(stop)")
		return true
	end
	
	COutLine(kC_Debug, "Music.Command(%s) was not recognized.", cmd)
	
end

function Music.SaveState(self)
	local state = {}
	
	self.think = nil
	
	if (self.name and self.active) then
		state.active = self.name
	end
	
	return state
	
end

function Music.LoadState(self, state)

	if (state.active) then
	-- spin up another one
		self.name = nil
		self.args = {state.active}
		self:Play()
	else
		if (self.active) then
			self.active:FadeOutAndStop(1)
			self.active = nil
			self.name = nil
			self.think = nil
		end
		World.PostEvent("shiphum fadeto 1 1")
	end

end

imuse = Music
