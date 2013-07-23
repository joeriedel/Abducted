-- Light.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

kLightStyle_Diffuse = 1
kLightStyle_Specular = 2
kLightStyle_CastShadows = 4

kObjectLightingFlag_None = 0
kObjectLightingFlag_CastShadows = 1

kLightInteractionFlag_None = 0
kLightInteractionFlag_World = 1
kLightInteractionFlag_Player = 2
kLightInteractionFlag_Objects = 4
kLightInteractionFlag_All = bit.bor(kLightInteractionFlag_World, kLightInteractionFlag_Player, kLightInteractionFlag_Objects)

kLightEntityFlag_Diffuse = 1
kLightEntityFlag_Specular = 2
kLightEntityFlag_CastShadows = 4
kLightEntityFlag_AffectWorld = 8
kLightEntityFlag_AffectPlayer = 16
kLightEntityFlag_AffectObjects = 32
kLightEntityFlag_AffectAll = bit.bor(kLightEntityFlag_AffectWorld, kLightEntityFlag_AffectPlayer, kLightEntityFlag_AffectObjects)

Light = Entity:New()

function Light.Spawn(self)
	COutLine(kC_Debug, "Light:Spawn")
	Entity.Spawn(self)
	
	self.light = World.CreateDynamicLight()
	
	local diffuseColor = Vec3ForString(self.keys.diffuseColor, {1,1,1})
	self.light:SetDiffuseColor(diffuseColor)
	
	local specularColor = Vec3ForString(self.keys.specularColor, {1,1,1})
	self.light:SetSpecularColor(specularColor)
	
--	local shadowColor = {0,0,0,1}
--	self.light:SetShadowColor(shadowColor)
	
	self.light:SetPos(self:WorldPos())
	
	local radius = NumberForString(self.keys.radius, 400)
	self.light:SetRadius(radius)
	
	self.intensity = NumberForString(self.keys.intensity, 1)
	self.light:SetIntensity(self.intensity)
	
	local flags = NumberForString(self.keys.flags, 59)
	local style = 0
	
	if (bit.band(flags, kLightEntityFlag_Diffuse) ~= 0) then
		style = bit.bor(style, kLightStyle_Diffuse)
	end
	
	if (bit.band(flags, kLightEntityFlag_Specular) ~= 0) then
		style = bit.bor(style, kLightStyle_Specular)
	end
	
	if (bit.band(flags, kLightEntityFlag_CastShadows) ~= 0) then
		style = bit.bor(style, kLightStyle_CastShadows)
	end
	
	self.light:SetStyle(style)
	
	style = StringForString(self.keys.style, "none")
	if (style == "pslow") then
        self:SlowPulse() --SlowPulse()
    elseif (style == "pfast") then
        self:FastPulse() --FastPulse()
    elseif (style == "fslow") then
        self:SlowFlash() --SlowFlash()
    elseif (style == "ffast") then
        self:FastFlash() --Fastflash()
	elseif (style == "wiflicker") then -- intermittent flickering
		self:WeakIntermittentFlicker()
	elseif (style == "siflicker") then -- intermittent flickering
		self:StrongIntermittentFlicker()
    end
	
	local interactions = 0
	
	if (bit.band(flags, kLightEntityFlag_AffectWorld) ~= 0) then
		interactions = bit.bor(interactions, kLightInteractionFlag_World)
	end
	
	if (bit.band(flags, kLightEntityFlag_AffectPlayer) ~= 0) then
		interactions = bit.bor(interactions, kLightInteractionFlag_Player)
	end
	
	if (bit.band(flags, kLightEntityFlag_AffectObjects) ~= 0) then
		interactions = bit.bor(interactions, kLightInteractionFlag_Objects)
	end
	
	self.light:SetInteractionFlags(interactions)
	
	self.light:Link()
end

function Light.SlowPulse(self)
	local steps = {
		{ intensity = 0, time = 3 },
		{ intensity = self.intensity, time = 3 }
	}
	self.light:AnimateIntensity(steps, true)
end

function Light.FastPulse(self)
	local steps = {
		{ intensity = 0, time = .25 },
		{ intensity = self.intensity, time = .25 }
	}
	self.light:AnimateIntensity(steps, true)
end

function Light.SlowFlash(self)
	local steps = {
        { intensity = 0, time = 0 },
        { intensity = 0, time = .25 },
        { intensity = self.intensity, time = 0 },
        { intensity = self.intensity, time = .25 }
	}
	self.light:AnimateIntensity(steps, true)
end

function Light.FastFlash(self)
	local steps = {
        { intensity = 0, time = 0 },
        { intensity = 0, time = .125 },
        { intensity = self.intensity, time = 0},
        { intensity = self.intensity, time = .125}
	}
	self.light:AnimateIntensity(steps, true)
end

function Light.AddFlickerCommands(self, flickerTable, duration, intensityScale)
	-- This function will add flickering intensities and times to "flickerTable"
	-- intensityScale will control how strong the perceived flicker is
	-- FloatRand(min, max) returns a random float between min and max
	
	-- time delays much lower than 0.033 probably won't get rendered (1/30th of a second).
	local totalTime = 0
	local timeStep = 0.017 -- base duration of times
	
	while (totalTime < duration) do
		local step = {}
		step.intensity = self.intensity * FloatRand(intensityScale, 1)
		step.time = timeStep * FloatRand(0.7, 1.5)
		totalTime = totalTime + step.time
		table.insert(flickerTable, step)
	end
end

function Light.WeakIntermittentFlicker(self)

	self.nextFlicker = 1
	self.flickers = {}
	self.mode = "hold"
	
	for i=1,5 do -- generate 5 sets of randoms flickers
		local steps = {}
		self:AddFlickerCommands(steps, 2, 0.8)
		table.insert(self.flickers, steps)
	end
	
	self.light:AnimateIntensity({{intensity=self.intensity, time=0}}, false) -- light is on at full to start
	
	self.think = Light.FlickerThink
	self:SetNextThink(FloatRand(2, 5))
	
end

function Light.StrongIntermittentFlicker(self)

	self.nextFlicker = 1
	self.flickers = {}
	self.mode = "hold"
	
	for i=1,5 do -- generate 5 sets of randoms flickers
		local steps = {}
		self:AddFlickerCommands(steps, 2, 0.2) -- NOTE: Strong flicker means the light gets closer to black (intensityScale is 0.2)
		table.insert(self.flickers, steps)
	end
	
	self.light:AnimateIntensity({{intensity=self.intensity, time=0}}, false) -- light is on at full to start
	
	self.think = Light.FlickerThink
	self:SetNextThink(FloatRand(2, 5))
	
end

function Light.FlickerThink(self)

	if (self.mode == "hold") then
		COutLine(kC_Debug, "Light - Flicker")
		self.mode = "flicker"
		if (self.nextFlicker > #self.flickers) then
			self.nextFlicker = 1
		end
		self.light:AnimateIntensity(self.flickers[self.nextFlicker], true)
		self.nextFlicker = self.nextFlicker + 1
		self:SetNextThink(FloatRand(2, 4))
	else
		COutLine(kC_Debug, "Light - Hold")
		self.mode = "hold"
		self.light:AnimateIntensity({{intensity=self.intensity, time=0}}, false) -- light is on at full
		self:SetNextThink(FloatRand(5, 7))
	end

end

function Light.OnEvent(self, cmd, args)
	COutLineEvent("Light", cmd, args)
end

info_dynlight = Light

