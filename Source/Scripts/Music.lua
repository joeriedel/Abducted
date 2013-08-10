-- Music.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Music = Class:New()

function Music.Spawn(self)
	Music.entity = self
	self.queue = LL_New()
	
	local io = {
		Save = function()
			return self:SaveState()
		end,
		Load = function(s, x)
			return self:LoadState(x)
		end
	}
	
	GameDB.PersistentObjects["music"] = io
	
	self.think = Music.Next
	self:SetNextThink(0)
	
end

function Music.Next(self)

	if (self.f) then
		self.f(self)
		self.f = nil
	end
	
	local head = LL_Pop(self.queue)
	if (head) then
		self.f = head.f
	end
	
	self:Check()

end

function Music.Play(self, cmd, args)
	
	if (self.name and (self.name == args[1])) then
		-- already playing
		return
	end
	
	self:Stop() -- fade out active music
	
	self.cmd = cmd
	self.name = args[1]
		
	if (GameDB.loadingCheckpoint) then
		self.active = World.Load(args[1], 1, false) -- blocking load
	else
		self.active = World.Load(args[1])
	end
	
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
	
	World.PostEvent("shiphum fadeto 0.5 1")
end

function Music.Check(self)

	if (self.active) then
		if (not self.active:Playing()) then
			World.PostEvent("shiphum fadeto 1 1")
			self.active = nil
			self.name = nil
			self.cmd = nil
		end
	end

end

function Music.Stop(self)
	self.name = nil
	self.cmd = nil
	self.args = nil
	if (self.active) then
		if (self.active:Playing()) then
			local sound = self.active
			local f = function()
				sound = nil
			end
			self.active:FadeOutAndStop(1)
			self.active = nil
			World.gameTimers:Add(f, 1.1)
		else
			self.active = nil
		end
	end
end

function Music.Command(self, cmd, args, event)
		
	if (cmd == "play") then
		local f = function (self)
			Music.Play(self, event, args)
		end
		LL_Append(self.queue, {f=f})
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
	
	if (self.name and self.active) then
		state.cmd = self.cmd
	end
	
	return state
	
end

function Music.LoadState(self, state)

	self.queue = LL_New()

	if (state.cmd) then
	-- spin up another one
		self.name = nil
		local f = function(self)
			local x = Tokenize(state.cmd)
			table.remove(x, 1)
			Music.Play(self, state.cmd, x)
		end
		LL_Append(self.queue, {f=f})
	else
		if (self.active) then
			self.active:FadeOutAndStop(1)
			self.active = nil
			self.name = nil
			self.cmd = nil
		end
		local f = function()
			World.PostEvent("shiphum fadeto 1 1")
		end
		-- cannot post event here
		World.globalTimers:Add(f, 0)
	end

end

imuse = Music
