-- SoundEmitter.lua
-- Copyright (c) 2010 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel (joeriedel@hotmail.com)
-- See Crow/LICENSE for licensing terms

SoundEmitter = Entity:New()

function SoundEmitter.Spawn(self)

	local file = StringForString(self.keys.sound, nil)
	if file == nil then
		return
	end
	
	self.sound = World.LoadSound(file)
	if self.sound == nil then
		return
	end

	local maxDistance = NumberForString(self.keys.maxDistance, 200)
	local channel = StringForString(self.keys.channel, "Ambient")
	self.priority = NumberForString(self.keys.priority, 0)
	self.on = StringForString(self.keys.initial_state, "on") == "on"
	self.positional = BoolForString(self.keys.positional, true)
	self.volume = Clamp(NumberForString(self.keys.volume, 1), 0, 1)
	self.fadeTime = nil
	self.enabled = false
	
	if (channel == "UI") then
		self.channel = kSoundChannel_UI
	elseif (channel == "Ambient") then
		self.channel = kSoundChannel_Ambient
	elseif (channel == "FX") then
		self.channel = kSoundChannel_FX
	else
		self.channel = kSoundChannel_Music
	end
	
	self.sound:FadeVolume(self.volume, 0)
	self.sound:SetMaxDistance(maxDistance)
	self.sound:SetRefDistance(maxDistance/3)
	self.sound:SetLoop(BoolForString(self.keys.loop, true))
	
	if self.positional then
		self:AttachSound(self.sound)
	end
	
	-- start playing music during loading screen
	if (self.channel == kSoundChannel_Music) and (not self.positional) and (self.on) then
		self:Trigger()
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

function SoundEmitter.PostSpawn(self)
	self:Trigger()
end

function SoundEmitter.Enable(self, enable) -- callback from native code

	self.enabled = enable
	self:Trigger();
	
end

function SoundEmitter.OnEvent(self, cmd, args)
	COutLineEvent("SoundEmitter", cmd, args)
	
	if cmd == "start" or cmd == "play" then
		self.on = true
		self.fadeTime = nil
	elseif cmd == "fadein" then
		self.on = true
		self.fadeTime = tonumber(args)
		self.fadeLevel = 1
	elseif cmd == "fadeout" then
		self.on = false
		self.fadeTime = tonumber(args)
	elseif cmd == "fadeto" then
		args = Tokenize(args)
		self.volume = tonumber(args[1])
		self.fadeTime = tonumber(args[2])
		self.sound:FadeVolume(self.volume, self.fadeTime)
		return true
	elseif cmd == "rewind" then	
		self.on = false
		if self.sound then
			self.sound:Rewind()
		end
	else
		self.on = false
		self.fade = nil
	end
	
	self:Trigger()
	return true
	
end

function SoundEmitter.Trigger(self)

	if not self.sound then
		return
	end
	
	local playing = self.sound:Playing()
	local on = (self.enabled or (not self.positional)) and self.on
	
	if (not playing) and on then
		-- fading in?
		if (self.fadeTime) then
			self.sound:FadeVolume(0, 0)
			self.sound:FadeVolume(self.volume, self.fadeTime)
			self.fadeTime = nil
		else
			self.sound:FadeVolume(self.volume, 0)
		end
		self.sound:Play(self.channel, self.priority)
	elseif playing and (not on) then
		if (self.fadeTime) then
			self.sound:FadeOutAndStop(self.fadeTime)
			self.fadeTime = nil
		else
			self.sound:Stop()
		end
	end

end

function SoundEmitter.SaveState(self)
	local state = {
		on = tostring(self.on)
	}
	
	state.volume = tostring(self.volume)
		
	return state
end

function SoundEmitter.LoadState(self, state)

	self.fadeTime = 1 -- smooth transitions
	self.volume = tonumber(state.volume)
	
	local on = state.on == "true"
	
	if (self.on ~= on) then
		self.on = on
		self:Trigger()
		if (not on) then
			if (self.sound) then
				self.sound:Rewind()
			end
		end
	end

end


info_sound_emitter = SoundEmitter
