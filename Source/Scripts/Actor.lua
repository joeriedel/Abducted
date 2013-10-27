-- Actor.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Actor
-----------------------------------------------------------------------------]]

Actor = Entity:New()
Actor.GhostMaxParticles = 45
Actor.GhostParticleType = "FX/ghostparticles"
Actor.GhostParticleSpread = 0
Actor.GhostParticleVolume = {5, 5, 5}
Actor.GhostPPS = 10

function Actor.Spawn(self)

    COutLine(kC_Debug, "Actor:Spawn")
    Entity.Spawn(self)
    
    if (BoolForString(self.keys.cast_shadows, false)) then
		self:SetLightingFlags(kObjectLightingFlag_CastShadows)
	end
	
	self:SetLightInteractionFlags(kLightInteractionFlag_Objects)
    
    self.visible = false
    
    if (self.keys.model) then
        self.model = LoadModel(self.keys.model)
        self.model.dm = self:AttachDrawModel(self.model)
        if (self.model.SetRootController and self.keys.idle) then
			self.state = self.keys.idle
            self.model:BlendToState(self.keys.idle)
        end 
        if (self.keys.scale) then
			self.model.dm:ScaleTo(Vec3ForString(self.keys.scale), 0)
        end
        if (self.keys.angles) then
			self:SetRotation(Vec3ForString(self.keys.angles))
		end
        self.visible = BoolForString(self.keys.visible, true)
        self.model.dm:SetVisible(self.visible)
        
        self:SetMins(Vec3ForString(self.keys.mins), {-32, -32, -32})
		self:SetMaxs(Vec3ForString(self.keys.maxs), { 32,  32,  32})
		self:SetShadowMins(self:Mins())
		self:SetShadowMaxs(self:Maxs())
        self.model.dm:SetBounds(self:Mins(), self:Maxs())
        
        self:AddGhostParticles(self.model, self.keys.model)
        self:EnableGhostParticles(self.visible)
        
        self:SetOccupantType(kOccupantType_BBox)
		self:Link()
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

function Actor.EnableGhostParticles(self, enable, fade, time)
	if (self.ghostParticles) then
		for k,v in pairs(self.ghostParticles) do
			if (enable ~= nil) then
				v.dm:SetVisible(enable)
			end
			if (fade) then
				v.dm:BlendTo(fade, time)
			end
		end
	end
end

function Actor.OnEvent(self, cmd, args)
	COutLineEvent("Actor", self.keys.targetname, cmd, args)
	
	if (cmd == "show") then
		self.visible = true
		if (self.model) then
			self.model.dm:SetVisible(true)
			self.model.dm:BlendTo({1,1,1,1}, 0)
			self:EnableGhostParticles(true, {1,1,1,1}, 0)
		end
		return true
	elseif (cmd == "hide") then
		self.visible = false
		if (self.model) then
			self.model.dm:SetVisible(false)
			self:EnableGhostParticles(false)
		end
		return true
	elseif (cmd == "fadein") then
		if (not self.visible) then
			self.visible = true
			if (self.model) then
				self.model.dm:BlendTo({1,1,1,0}, 0)
				self.model.dm:SetVisible(true)
				self:EnableGhostParticles(true, {1,1,1,0}, 0)
			end
		end
		if (self.model) then
			self.model.dm:BlendTo({1,1,1,1}, tonumber(args))
			self:EnableGhostParticles(nil, {1,1,1,1}, tonumber(args))
		end
		return true
	elseif (cmd == "fadeout") then
		self.visible = false
		if (self.model) then
			self.model.dm:BlendTo({1,1,1,0}, tonumber(args))
			self:EnableGhostParticles(nil, {1,1,1,0}, tonumber(args))
		end
		return true
	elseif (cmd == "fadetoblack") then
		self.visible = false
		if (self.model) then
			self.model.dm:BlendTo({0,0,0,0}, tonumber(args))
			self:EnableGhostParticles(nil, {0,0,0,0}, tonumber(args))
		end
		return true
	elseif (cmd == "play") then
		if (self.model.BlendToState) then
			self.model:BlendToState(args)
		end
		return true
	end
	
	return false
end

function Actor.AddGhostParticles(self, model, modelname)

	if (modelname ~= "Characters/Ghost1") then
		return
	end
	
	local bones = {
		"Bip01",
		"Spine1",
		"Spine2",
		"Spine3",
		"Spine4",
		"Shoulder_right",
		"Arm_right",
		"Forearm_right",
		"Hand_right",
		"Shoulder_left",
		"Arm_left",
		"Forearm_left",
		"Hand_left",
		"Neck",
		"Head",
		"Hip_right",
		"Leg_right",
		"Calf_right",
		"Foot_right",
		"Hip_left",
		"Leg_left",
		"Calf_left",
		"Foot_left"
	}
	
	self.ghostParticles = {}
	
	-- because emitters are set to never cull (i.e. particles are always being processed
	-- regardless of visiblity) then we must make the skel they are attached to always tick
	-- so the emitters have valid bone positions
	
	model.dm:SetForceTick(true)
	
	local mins = self:Mins()
	local maxs = self:Maxs()
	local dir = {0,0,1}
	
	for k,v in pairs(bones) do
	
		local boneIdx = model.dm:FindBone(v)
		if (boneIdx < 0) then
			error(string.format("Error: ghost has no bone named '%s'.", v))
			return
		end
	
		local emitter = World.Load(Actor.GhostParticleType)
		emitter:SetMaxParticles(Actor.GhostMaxParticles)
				
		emitter.dm = self:AttachDrawModel(emitter)
		emitter.dm:SetBounds(mins, maxs)
		-- note: since we are attached to a bone this only effects the direction vector.
		emitter.dm:SetPositionMode(kParticleEmitterDrawModelPositionMode_World)
		emitter.dm:SetLocalDir(dir)
		emitter.dm:SetCullMode(kParticleEmitterDrawModelCullMode_None)
		
		emitter:SetSpread(Actor.GhostParticleSpread)
		emitter:SetVolume(Actor.GhostParticleVolume)
		emitter:SetPPS(Actor.GhostPPS)
		
		model.dm:AttachChildToBone(emitter.dm, boneIdx)
		
		table.insert(self.ghostParticles, emitter)
	end

end

function Actor.SaveState(self)
	
	local state = {
		visible = tostring(self.visible)
	}
	
	if (self.state) then
		state.state = self.state
	end
	
	return state
end

function Actor.LoadState(self, state)
	
	if (state.visible == "true") then
		self.model.dm:SetVisible(true)
		self.model.dm:BlendTo({1,1,1,1}, 0)
		self:EnableGhostParticles(nil, {1,1,1,1}, 0)
	else
		self.model.dm:SetVisible(false)
	end
	
	if (state.state) then
		self.model:BlendToState(state.state)
	end
	
end

info_actor = Actor

