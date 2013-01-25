-- ManipulatableObject.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ManipulatableObject = Entity:New()
ManipulatableObject.Objects = LL_New()

function ManipulatableObject.Spawn(self)
	COutLine(kC_Debug, "Manipulatable:Spawn(%s)", StringForString(self.keys.model, "<NULL>"))
	
	self.model = LoadSkModel(self.keys.model)
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.vision = self.model.dm:CreateInstance()
	self:AttachDrawModel(self.model.vision) -- manipulate vision model
	self.model.vision:BlendTo({1,1,1,0}, 0)
	
	MakeAnimatable(self)
	self:SetOccupantType(kOccupantType_BBox)
	
	local angle = NumberForString(self.keys.angle, 0)
	
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	self:SetAngles(angleVertex)
	
	self.skillRequired = NumberForString(self.keys.skill, 0)
	self.manipulateWindow = NumberForString(self.keys.manipulate_window, 5)
	
	self.enabled = false
	
	self:LoadSounds()
	self:Dormant()
end

function ManipulatableObject.LoadSounds(self)

	self.sounds = {}
	
	if (self.keys.dormant_sound) then
		self.sounds.Dormant = World.LoadSound(self.keys.dormant_sound)
		self.sounds.Dormant:SetLoop(true)
	end
	
	if (self.keys.awaken_sound) then
		self.sounds.Awaken = World.LoadSound(self.keys.awaken_sound)
	end
	
	if (self.keys.idle_sound) then
		self.sounds.Idle = World.LoadSound(self.keys.idle_sound)
		self.sounds.Idle:SetLoop(true)
	end
	
	if (self.keys.attack_sound) then
		self.sounds.Attack = World.LoadSound(self.keys.attack_sound)
	end
	
	if (self.keys.hit_sound) then
		self.sounds.Hit = World.LoadSound(self.keys.hit_sound)
	end
	
	if (self.keys.death_sound) then
		self.sounds.Death = World.LoadSound(self.keys.death_sound)
	end
	
	local maxDistance = NumberForString(self.keys.sound_max_distance, "400")
	
	for k,v in pairs(self.sounds) do
		v:SetMaxDistance(maxDistance)
		v:SetRefDistance(maxDistance/2)
		self:AttachSound(v)
	end

end

function ManipulatableObject.OnEvent(self, cmd, args)
	if (cmd == "activate") then
		self:Awaken()
	end
end

function ManipulatableObject.Dormant(self)
	self:PlayAnim("dormant", self.model)
	if (self.sounds.Dormant) then
		self.sounds.Dormant:Play(kSoundChannel_FX, 0)
	end
end

function ManipulatableObject.Awaken(self)
	self:PlayAnim("awaken", self.model).Seq(ManipulatableObject.Idle)
	if (self.sounds.Dormant) then
		self.sounds.Dormant:FadeVolume(0, 1)
	end
	if (self.sounds.Awaken) then
		self.sounds.Awaken:Play(kSoundChannel_FX, 0)
	end
end

function ManipulatableObject.Idle(self)
	self:PlayAnim("idle", self.model)
	if (self.sounds.Idle) then
		self.sounds.Idle:Play(kSoundChannel_FX, 0)
	end
	
	if (not self.enabled) then
		self.enabled = true
		LL_Append(ManipulatableObject.Objects, {entity=self})
		if (Game.entity.manipulate) then -- show ourselves
			ManipulatableObject.NotifyManipulate(true)
		end
	end
end

function ManipulatableObject.NotifyManipulate(enabled)

	local time = 0.5
	local hidden = {1,1,1,0}
	
	if (enabled) then
		time = 0.15
	end
	
	local f = function (x)
		local rgba
		if (enabled) then
			rgba = x.entity:SelectColor()
		else
			rgba = hidden
		end
		
		x.entity.model.vision:BlendTo(rgba, time)
	end
	
	LL_Iterate(ManipulatableObject.Objects, f)

end

function ManipulatableObject.SelectColor(self)
		
	if (self.skillRequired > PlayerSkills.Manipulate+1) then
		return {1,0,0,1}
	elseif (self.skillRequired <= PlayerSkills.Manipulate) then
		return {0,1,0,1}
	end
	
	return {1,1,0,1}
end

--[[---------------------------------------------------------------------------
	Tentacles
-----------------------------------------------------------------------------]]

Tentacle = ManipulatableObject:New()

function Tentacle.Spawn(self)
	ManipulatableObject.Spawn(self)
	
	self:SetMins({-24, -24, -48+64})
	self:SetMaxs({ 24,  24,  48+64})
	self.model.dm:SetScale({0.4, 0.4, 0.4})
	self.model.dm:SetAngles({0, -90, 180})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	self.model.vision:SetScale({0.4, 0.4, 0.4})
	self.model.vision:SetAngles({0, -90, 180})
--	self.model.vision:SetPos({0, 100, 0})
	self.model.vision:SetBounds(self:Mins(), self:Maxs())
	
	self:Link() -- kMoveType_None
end

info_tentacle = Tentacle