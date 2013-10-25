-- Mine.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Detonatable Mine
-----------------------------------------------------------------------------]]

Mine = Entity:New()
Mine.kAccel = -75
Mine.kScanArea = {
	mins={-25, -25, 0},
	maxs={25, 25, 25}
}
Mine.kBlastArea = {
	mins={-50, -50, -50},
	maxs={ 50,  50,  50}
}

function Mine.Spawn(self)

	COutLine(kC_Debug, "Mine:Spawn")
	
	Entity.Spawn(self)
	self:SetLightInteractionFlags(kLightInteractionFlag_Objects)

	self.model = World.Load("Objects/mine")
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:ScaleTo({0.25, 0.25, 0.25}, 0)
	
	self.sfx = {
		Activate = World.LoadSound("Audio/mine_activate"),
		Hum = World.LoadSound("Audio/mine_hum"),
		Armed = World.LoadSound("Audio/mine_armed"),
		Tripped = World.LoadSound("Audio/mine_tripped"),
		Explode = World.LoadSound("Audio/mine_explode")
	}
	
	for k,v in pairs(self.sfx) do
		self:AttachSound(v)
	end
	
	self.sfx.Hum:SetLoop(true)
	
	self:SetMins({-48, -48, 0})
	self:SetMaxs({ 48,  48, 32})
	self:SetShadowMins(self:Mins())
	self:SetShadowMaxs(self:Maxs())
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()
	
	self.pos = self:WorldPos()
end

function Mine.DropToFloor(self, floorZ)

	self.vel = 0
	self.floorZ = floorZ
	
	self.think = Mine.DropThink
	self:SetNextThink(0)
	
	self.sfx.Activate:Play(kSoundChannel_FX, 0)
	self.sfx.Hum:FadeVolume(0, 0)
	self.sfx.Hum:Play(kSoundChannel_FX, 0)
	self.sfx.Hum:FadeVolume(1, 1)
	
end

function Mine.DropThink(self, dt)
	self.vel = self.vel + (Mine.kAccel * dt)
	self.pos[3] = self.pos[3] + (self.vel*dt)
	
	if (self.pos[3] <= self.floorZ) then
		self.pos[3] = self.floorZ
		self.think = Mine.ArmThink
		self:SetNextThink(2)
	end
	
	self:SetOrigin(self.pos)
	self:SetWorldPos(self.pos)
	self:Link()
end

function Mine.ArmThink(self)
	self.sfx.Armed:Play(kSoundChannel_FX, 0)
	local materials = World.playerPawn.mineMaterials
	self.model.dm:ReplaceMaterial(materials.Default, materials.Armed)
	
	self.scanMins = VecAdd(self.pos, Mine.kScanArea.mins)
	self.scanMaxs = VecAdd(self.pos, Mine.kScanArea.maxs)
	self.blastMins = VecAdd(self.pos, Mine.kBlastArea.mins)
	self.blastMaxs = VecAdd(self.pos, Mine.kBlastArea.maxs)
	
	self.think = Mine.Scan
	self:SetNextThink(0.5)
end

function Mine.Scan(self)

	local ents = World.BBoxTouching(
		self.scanMins,
		self.scanMaxs,
		bit.bor(kEntityClass_Player, kEntityClass_Monster)
	)
	
	if (ents) then
		self:Trip()
	end

end

function Mine.Trip(self)
	self.think = Mine.Explode
	self:SetNextThink(FloatRand(0.7, 1.2))
	self.sfx.Tripped:Play(kSoundChannel_FX, 0)
	
	local materials = World.playerPawn.mineMaterials
	self.model.dm:ReplaceMaterial(materials.Armed, materials.Tripped)
end

function Mine.Explode(self)
	self.sfx.Explode:Play(kSoundChannel_FX, 0)
	self.sfx.Hum:Stop()
	self.model.dm:SetVisible(false)
	
	local ents = World.BBoxTouching(
		self.blastMins,
		self.blastMaxs,
		bit.bor(kEntityClass_Player, kEntityClass_Monster)
	)
	
	if (ents) then
		for k,v in pairs(ents) do
			if (v.Kill) then
				v:Kill(self)
			end
		end
	end
	
	self.think = Mine.Remove
	self:SetNextThink(1)
	
	World.playerPawn:RemoveMine(self)
end

function Mine.Remove(self)
	self.think = nil
	self:Delete() -- mark for gc
end

info_mine = Mine