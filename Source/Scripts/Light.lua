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

kLightEntityFlag_Diffuse = 1
kLightEntityFlag_Specular = 2
kLightEntityFlag_CastShadows = 4
kLightEntityFlag_AffectWorld = 8
kLightEntityFlag_AffectPlayer = 16
kLightEntityFlag_AffectObjects = 32

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
	
	local radius = NumberForString(self.keys.radius, 400) * 0.5
	self.light:SetRadius(radius)
	
	local brightness = NumberForString(self.keys.brightness, 2)
	self.light:SetBrightness(brightness)
	
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

function Light.OnEvent(self, cmd, args)
	COutLineEvent("Light", cmd, args)
end

info_dynlight = Light

