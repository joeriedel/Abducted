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
	
	self.light:SetPos(self:WorldPos())
	
	local radius = NumberForString(self.keys.radius, 400)
	self.light:SetRadius(radius)
	
	self.intensity = NumberForString(self.keys.intensity, 1)
	self.light:SetIntensity(math.max(0, self.intensity))
	self.light:SetShadowWeight(self.intensity)
	
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
	
	self:SetStyle(StringForString(self.keys.style, "none"))
	
	local state = StringForString(self.keys.state, "on")
	
	if (state == "on") then
		self:TurnOn(0)
	else
		self:TurnOff(0)
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

function Light.TurnOn(self, time)
	if (self.state == "on") then
		return
	end
	
	self.state = "on"
	self.light:FadeTo(1, time)
	
end

function Light.TurnOff(self, time)
	if (self.state == "off") then
		return
	end
	
	self.state = "off"
	self.light:FadeTo(0, time)
end

function Light.SetStyle(self, style)
	if (self.style == style) then
		return
	end
	
	if (style == "none") then
		self.think = nil
		self.light:SetIntensity(self.intensity)
	elseif (style == "pslow") then
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
	elseif (style == "lightning") then
		self:Lightning()
	else
		return false -- unrecognized style
    end
    
    self.style = style
    return true
end

function Light.SlowPulse(self)
	local steps = {
		{ intensity = 0, time = 3 },
		{ intensity = self.intensity, time = 3 }
	}
	self.think = nil
	self.light:AnimateIntensity(steps, true)
end

function Light.FastPulse(self)
	local steps = {
		{ intensity = 0, time = .25 },
		{ intensity = self.intensity, time = .25 }
	}
	self.think = nil
	self.light:AnimateIntensity(steps, true)
end

function Light.SlowFlash(self)
	local steps = {
        { intensity = 0, time = 0 },
        { intensity = 0, time = .25 },
        { intensity = self.intensity, time = 0 },
        { intensity = self.intensity, time = .25 }
	}
	self.think = nil
	self.light:AnimateIntensity(steps, true)
end

function Light.FastFlash(self)
	local steps = {
        { intensity = 0, time = 0 },
        { intensity = 0, time = .125 },
        { intensity = self.intensity, time = 0},
        { intensity = self.intensity, time = .125}
	}
	self.think = nil
	self.light:AnimateIntensity(steps, true)
end

function Light.AddLightningCommands(self, flickerTable, numFlashes, speed, holdTime, fadeOutTime, intensityScale)

	for i=1,numFlashes do
		
		local step = {
			intensity = self.intensity * FloatRand(intensityScale, 1)
		}
		
		step.time = FloatRand(speed[1], speed[2])
		
		table.insert(flickerTable, step)
		
		if (i == numFlashes) then
			-- hold this intenstiy
			step.time = FloatRand(holdTime[1], holdTime[2])
			table.insert(flickerTable, step)
			step.intensity = 0
			step.time = FloatRand(fadeOutTime[1], fadeOutTime[2])
			table.insert(flickerTable, step)
		end
	
	end

end

function Light.Lightning(self, singleShot)

	if (self.lighting == nil) then
		self.lighting = {}
		for i=1,5 do -- generate 5 sets of random lighting
			local steps = {}
			self:AddLightningCommands(
				steps,
				IntRand(12, 18),
				{0.05, 0.08},
				{0.5, 0.7},
				{0.3, 0.6},
				0
			)
			table.insert(self.lighting, steps)
		end
	end
	
	self.flickers = self.lighting
	self.nextFlicker = IntRand(1, #self.lighting)
	
	if (singleShot) then
		self.mode = "singleshot"
	else
		self.mode = "continuous"
	end
	
	self.think = Light.LightningThink
	self:think()
	
end

function Light.LightningThink(self)

	if (self.nextFlicker > #self.flickers) then
		self.nextFlicker = 1
	end
	self.light:AnimateIntensity(self.flickers[self.nextFlicker], false)
	
	if (self.mode == "singleshot") then
		self.think = nil
	else
		self.nextFlicker = self.nextFlicker + 1
		self:SetNextThink(FloatRand(3, 7))
	end
	
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

	if (self.wiflickers == nil) then
		self.wiflickers = {}
		for i=1,5 do -- generate 5 sets of random flickers
			local steps = {}
			self:AddFlickerCommands(steps, 2, 0.8)
			table.insert(self.wiflickers, steps)
		end
	end
	
	self.nextFlicker = 1
	self.flickers = self.wiflickers
	self.mode = "hold"
	
	self.light:AnimateIntensity({{intensity=self.intensity, time=0}}, false) -- light is on at full to start
	
	self.think = Light.FlickerThink
	self:SetNextThink(FloatRand(2, 5))
	
end

function Light.StrongIntermittentFlicker(self)

	if (self.siflickers == nil) then
		self.siflickers = {}
		for i=1,5 do -- generate 5 sets of random flickers
			local steps = {}
			self:AddFlickerCommands(steps, 2, 0.2) -- NOTE: Strong flicker means the light gets closer to black (intensityScale is 0.2)
			table.insert(self.siflickers, steps)
		end
	end
	
	self.nextFlicker = 1
	self.flickers = self.siflickers
	self.mode = "hold"
	
	self.light:AnimateIntensity({{intensity=self.intensity, time=0}}, false) -- light is on at full to start
	
	self.think = Light.FlickerThink
	self:SetNextThink(FloatRand(2, 5))
	
end

function Light.FlickerThink(self)

	if (self.mode == "hold") then
--		COutLine(kC_Debug, "Light - Flicker")
		self.mode = "flicker"
		if (self.nextFlicker > #self.flickers) then
			self.nextFlicker = 1
		end
		self.light:AnimateIntensity(self.flickers[self.nextFlicker], true)
		self.nextFlicker = self.nextFlicker + 1
		self:SetNextThink(FloatRand(2, 4))
	else
--		COutLine(kC_Debug, "Light - Hold")
		self.mode = "hold"
		self.light:AnimateIntensity({{intensity=self.intensity, time=0}}, false) -- light is on at full
		self:SetNextThink(FloatRand(5, 7))
	end

end

function Light.OnEvent(self, cmd, args)
	COutLineEvent("Light", cmd, args)
	
	if (cmd == "on") then
		self:TurnOn(tonumber(args))
		return true
	elseif (cmd == "off") then
		self:TurnOff(tonumber(args))
		return true
	elseif (cmd == "style") then
		self:SetStyle(args)
		return true
	elseif (cmd == "flashlightning") then
		self:Lightning(true)
		return rtue
	end
end

function Light.SaveState(self)
	local state = {
		state = self.state,
		style = self.style
	}
	
	return state
end

function Light.LoadState(self, state)
	self.state = nil
	self.style = nil
	self.think = nil
	
	if (state.state == "on") then
		self:TurnOn(0)
	else
		self:TurnOff(0)
	end
	
	self:SetStyle(state.style)
end

info_dynlight = Light

