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

function Music.Play(self, cmd, args)
	self.think = nil

	if (self.name and (self.name == args[1])) then
		-- already playing
		return
	end
	
	self:Stop() -- fade out active music
	if (GameDB.loadingCheckpoint) then
		self.active = World.Load(args[1], 1, false) -- blocking load
	else
		self.active = World.Load(args[1])
	end
	self.name = args[1]
	self.cmd = cmd
	
	local fade = not FindArrayElement(args, "fade=false")
	local loop = FindArrayElement(args, "loop=true")
	
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
			self.cmd = nil
		end
	else
		self.think = nil
	end

end

function Music.Stop(self)
	self.name = nil
	self.cmd = nil
	if (self.active) then
		if (self.active:Playing()) then
			local sound = self.active
			local f = function()
				sound = nil
			end
			self.active:FadeOutAndStop(1)
			self.active = nil
			World.gameTimers:Add(f, 1.1, true)
		else
			self.active = nil
		end
	end
end

function Music.Command(self, cmd, args, event)
		
	if (cmd == "play") then
		self.think = function (self)
			Music.Play(self, event, args)
		end
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
		state.active = self.cmd
	end
	
	return state
	
end

function Music.LoadState(self, state)

	if (state.active) then
	-- spin up another one
		self.name = nil
		local x = Tokenize(state.active)
		table.remove(x, 1)
		self:Play(state.active, x)
	else
		if (self.active) then
			self.active:FadeOutAndStop(1)
			self.active = nil
			self.name = nil
			self.cmd = nil
			self.think = nil
		end
		World.PostEvent("shiphum fadeto 1 1")
	end

end

imuse = Music
