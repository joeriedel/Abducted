-- Music.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Music = Class:New()

function Music.Spawn(self)
	Music.entity = self
end

function Music.Play(self)
	if (self.name and (self.name == self.args[1])) then
		self.think = nil
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
	self.think = nil
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

imuse = Music
