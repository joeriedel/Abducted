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
	
	if channel == "UI" then
		self.channel = kSoundChannel_UI
	elseif channel == "Ambient" then
		self.channel = kSoundChannel_Ambient
	elseif channel == "FX" then
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
	
end

function SoundEmitter.OnLevelStart(self)
	self:Trigger()
end

function SoundEmitter.Enable(self, enable) -- callback from native code

	self.enabled = enable
	self:Trigger();
	
end

function SoundEmitter.OnEvent(self, cmd, args)

	if cmd == "start" or cmd == "play" then
		self.on = true
		self.fadeTime = nil
	elseif cmd == "fadeIn" then
		self.on = true
		self.fadeTime = tonumber(args)
	elseif cmd == "fadeOut" then
		self.on = false
		self.fadeTime = tonumber(args)
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
		self.think = nil
	elseif playing and (not on) then
		if (self.fadeTime) then
			self.sound:FadeVolume(0, self.fadeTime)
			self.fadeTime = nil
		else
			self.sound:Stop()
			self.think = nil
		end
	end

end


info_sound_emitter = SoundEmitter
