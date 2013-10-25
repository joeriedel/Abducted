-- ParticleEmitter.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ParticleEmitter = Entity:New()

function ParticleEmitter.Spawn(self)

	COutLine(kC_Debug, "ParticleEmitter:Spawn")
	Entity.Spawn(self)
	
	self.on = StringForString(self.keys.initial_state, "on") == "on"
	self.pps = NumberForString(self.keys.pps, 0)
	
	local mins = Vec3ForString(self.keys.mins, {-32, -32, -32})
	local maxs = Vec3ForString(self.keys.maxs, { 32,  32,  32})
	
	local dir = Vec3ForString(self.keys.dir, {0, 0, 1})
	
	if ((dir[1] ~= 0) or (dir[2] ~= 0) or (dir[3] ~= 0)) then
		dir = VecNorm(dir)
	else
		dir = {0, 0, 1}
	end
	
	local volume = Vec3ForString(self.keys.volume, {0,0,0})
	
	local spread = NumberForString(self.keys.spread, 0)
	local maxParticles = NumberForString(self.keys.max, 200)
	
	self:SetMins(mins)
	self:SetMaxs(maxs)
	self:SetShadowMins(self:Mins())
	self:SetShadowMaxs(self:Maxs())
		
	self.emitter = World.Load(self.keys.particle)
	self.emitter:SetMaxParticles(maxParticles) -- really important to do this before making the DM
	
	self.emitter.dm = self:AttachDrawModel(self.emitter)
	self.emitter.dm:SetBounds(mins, maxs)
	self.emitter.dm:SetLocalDir(dir)
	
	self.emitter:SetSpread(spread)
	self.emitter:SetVolume(volume)
	
	if (self.on) then
		self.emitter:SetPPS(self.pps)
	else
		self.emitter:SetPPS(0)
	end
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()

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

function ParticleEmitter.PostSpawn(self)

	if (self.keys.attach_entity) then
	
		local entity = World.FindEntityTargets(self.keys.attach_entity)
		if (entity) then
			entity = entity[1]
		else
			error(string.format("particle emitter can't find an entity named '%s' to attach to.", self.keys.attach_entity))
		end
	
		if ((entity.model == nil) or (entity.model.dm == nil) or (entity.model.dm.FindBone == nil)) then
			error(string.format("particle emitter attachment entity '%s' is not a skelmodel.", self.keys.attach_entity))
		end

		local boneIdx = entity.model.dm:FindBone(self.keys.attach_bone)
		if (boneIdx < 0) then
			error(string.format("particle emitter attachment entity '%s' does not have a bone named '%s'", self.keys.attach_entity, self.keys.attach_bone))
		end
		
		entity:AttachChild(self, entity.model.dm, boneIdx)
	
	end

end

function ParticleEmitter.OnEvent(self, cmd, args)

	COutLineEvent("ParticleEmitter", self.keys.targetname, cmd, args)
	
	if ((cmd == "enable") and args) then
		self.on = true
		self.emitter:SetPPS(self.pps)
		return true
	elseif (cmd == "disable") then
		self.on = false
		self.emitter:SetPPS(0)
		return true
	elseif ((cmd == "spawn") and args) then
		self.emitter:Spawn(tonumber(args))
		return true
	end

	return false
	
end

function ParticleEmitter.SaveState(self)
	local state = {
		on = tostring(self.on)
	}
	return state
end

function ParticleEmitter.LoadState(self, state)
	local wason = self.on
	
	self.on = state.on == "true"
	
	if (self.on ~= wason) then
		if (self.on) then
			self.emitter:SetPPS(self.pps)
		else
			self.emitter:SetPPS(0)
			self.emitter:Reset()
		end
	end
end

info_particle_emitter = ParticleEmitter